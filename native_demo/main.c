#define _POSIX_C_SOURCE 200809L

#include <alsa/asoundlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <opusrx/opusrx.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 0) {
    perror("socket");
    return -1;
  }

  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(5004);
  if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
    close(sock);
    return -1;
  }

  snd_pcm_t *handle;
  snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
  snd_pcm_set_params(handle, SND_PCM_FORMAT_S16_LE,
                     SND_PCM_ACCESS_RW_INTERLEAVED, 2, 48000, 1, 20000);

  player_t *player = init_player((player_config_t){.frame_size = 960,
                                                   .buffer_size = 20,
                                                   .queue_size = 40,
                                                   .channels = 2,
                                                   .sample_rate = 48000});

  uint8_t buffer[1500];
  int16_t pcm[960 * 2];
  rtp_packet_t pkt;
  ssize_t len;

  while ((len = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL)) > 0) {
    if (rtp_parse(buffer, len, &pkt) != 0) {
      perror("RTP parse");
      continue;
    }
    ingest_rtp(buffer, len, player);
    process_input(player);
    int samples = player_step(player, pcm);
    if (samples > 0) {
      int err = snd_pcm_writei(handle, pcm, samples);
      if (err < 0) {
        err = snd_pcm_recover(handle, err, 0);
      }
    }
  }
  return 0;
}
