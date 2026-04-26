#define _POSIX_C_SOURCE 200809L

#include <alsa/asoundlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <opusrx/opusrx.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

void *network_thread(void *arg) {
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 0) {
    perror("socket");
    return NULL;
  }
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(5004);
  if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
    close(sock);
    return NULL;
  }

  player_t *p = arg;
  uint8_t buffer[1500];

  printf("Network thread started\n");

  while (1) {
    ssize_t len = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
    if (len < 0) {
      perror("recvfrom");
      continue;
    }
    ingest_rtp(buffer, len, p);
  }
  return NULL;
}

void *playback_thread(void *arg) {
  snd_pcm_t *handle;
  if (snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
    perror("snd_pcm_open");
    return NULL;
  }
  if (snd_pcm_set_params(handle, SND_PCM_FORMAT_S16_LE,
                         SND_PCM_ACCESS_RW_INTERLEAVED, 2, 48000, 1,
                         20000) < 0) {
    perror("snd_pcm_set_params");
    return NULL;
  }

  player_t *p = arg;
  int16_t pcm[960 * 2];

  printf("Playback thread started\n");

  while (1) {
    int samples = render_frame(p, pcm);
    int err = snd_pcm_writei(handle, pcm, samples);
    if (err < 0) {
      err = snd_pcm_recover(handle, err, 0);
    }
  }
  return NULL;
}

int main() {
  setvbuf(stdout, NULL, _IONBF, 0);
  pthread_t net_thread, play_thread;
  player_t *player = init_player((player_config_t){.frame_size = 960,
                                                   .buffer_size = 15,
                                                   .queue_size = 30,
                                                   .channels = 2,
                                                   .sample_rate = 48000,
                                                   .target_depth = 8,
                                                   .err_threshold = 5,
                                                   .timeout = 200});
  pthread_create(&net_thread, NULL, network_thread, player);
  pthread_create(&play_thread, NULL, playback_thread, player);

  pthread_join(net_thread, NULL);
  pthread_join(play_thread, NULL);
  return 0;
}
