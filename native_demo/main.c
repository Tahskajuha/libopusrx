#define _POSIX_C_SOURCE 200809L
#define MAX_BUFFER_SIZE 1500
#define MAX_PCM_OUT_SIZE 11520
#define CFG_INT(group, key) g_key_file_get_integer(kf, group, key, NULL)
#define CFG_STR(group, key) g_key_file_get_string(kf, group, key, NULL)

#include <alsa/asoundlib.h>
#include <arpa/inet.h>
#include <glib.h>
#include <netinet/in.h>
#include <opusrx/opusrx.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/socket.h>

typedef struct {
  player_t *player;
  snd_pcm_t *handle;
} playback_arg_t;

typedef struct {
  player_t *player;
  int sock;
} network_arg_t;

void *network_thread(void *arg) {
  network_arg_t *data = arg;
  int s = data->sock;
  player_t *p = data->player;

  uint8_t buffer[MAX_BUFFER_SIZE];

  while (1) {
    ssize_t len = recvfrom(s, buffer, sizeof(buffer), 0, NULL, NULL);
    if (len < 0) {
      perror("recvfrom");
      continue;
    }
    ingest_rtp(buffer, len, p);
  }
  return NULL;
}

void *playback_thread(void *arg) {
  playback_arg_t *data = arg;
  player_t *p = data->player;
  snd_pcm_t *h = data->handle;

  int16_t pcm[MAX_PCM_OUT_SIZE];

  while (1) {
    snd_pcm_wait(h, 50);
    int samples = render_frame(p, pcm);
    int err = snd_pcm_writei(h, pcm, samples);
    if (err < 0) {
      err = snd_pcm_recover(h, err, 0);
      if (err < 0) {
        exit(1);
      }
    }
  }
  return NULL;
}

int create_sock(int port) {
  int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (s < 0) {
    g_error("socket");
  }
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);
  if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(s);
    g_error("bind");
  }
  return s;
}

int main(int argc, char *argv[]) {
  setvbuf(stdout, NULL, _IONBF, 0);
  pthread_t net_thread, play_thread;

  GKeyFile *kf = g_key_file_new();
  GError *error = NULL;
  if (argc < 2 ||
      !g_key_file_load_from_file(kf, argv[1], G_KEY_FILE_NONE, &error)) {
    g_error("Failed to load config: %s",
            error ? error->message
                  : "Unknown error; make sure config file exists");
  }

  int s = create_sock(CFG_INT("network", "port"));
  player_t *p = init_player((player_config_t){
      .frame_size = CFG_INT("player", "frame_size"),
      .buffer_size = CFG_INT("player", "buffer_size"),
      .queue_size = CFG_INT("player", "queue_size"),
      .channels = CFG_INT("player", "channels"),
      .sample_rate = CFG_INT("player", "sample_rate"),
      .target_depth = CFG_INT("player", "target_depth"),
      .err_threshold = CFG_INT("player", "err_threshold"),
      .timeout = CFG_INT("player", "timeout"),
      .warmup_frames = CFG_INT("player", "warmup_frames"),
  });
  snd_pcm_t *h;
  if (snd_pcm_open(&h, CFG_STR("alsa", "device"), SND_PCM_STREAM_PLAYBACK, 0) <
      0) {
    g_error("snd_pcm_open");
  }
  if (snd_pcm_set_params(
          h, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED,
          CFG_INT("player", "channels"), CFG_INT("player", "sample_rate"),
          CFG_INT("alsa", "soft_resample"),
          CFG_INT("alsa", "latency_us")) < 0) {
    g_error("snd_pcm_set_params");
  }

  network_arg_t network_arg = {.player = p, .sock = s};
  playback_arg_t playback_arg = {.player = p, .handle = h};

  pthread_create(&net_thread, NULL, network_thread, &network_arg);
  pthread_create(&play_thread, NULL, playback_thread, &playback_arg);

  pthread_join(net_thread, NULL);
  pthread_join(play_thread, NULL);
  return 0;
}
