#define _POSIX_C_SOURCE 200809L

#include "opusrx/opusrx.h"
#include "opusrx/primitives.h"
#include "pipeline.h"
#include "player.h"
#include <opus.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

player_t *init_player(const player_config_t cfg) {
  int err = 0;
  player_t *p = calloc(1, sizeof(player_t));
  if (!p)
    return NULL;
  p->jb = init_buffer(cfg.buffer_size);
  if (!p->jb) {
    free(p);
    return NULL;
  }
  p->q = init_queue(cfg.queue_size);
  if (!p->q) {
    destroy_buffer(p->jb);
    free(p);
    return NULL;
  }
  p->dec = opus_decoder_create(cfg.sample_rate, cfg.channels, &err);
  if (err != OPUS_OK) {
    destroy_buffer(p->jb);
    destroy_queue(p->q);
    free(p);
    return NULL;
  }
  p->channels = cfg.channels;
  p->sample_rate = cfg.sample_rate;
  p->window = (cfg.buffer_size - 1) * cfg.frame_size;
  p->frame_size = cfg.frame_size;
  p->target_depth = cfg.target_depth;
  p->err_threshold = cfg.err_threshold;
  p->timeout = cfg.timeout;
  return p;
}

int ingest_rtp(const uint8_t *buffer, size_t len, player_t *p) {
  if (!buffer || !p)
    return -1;
  rtp_packet_t pkt;
  if (rtp_parse(buffer, len, &pkt) != 0)
    return 0;
  pkt.payload = payload_copy(pkt.payload, pkt.payload_len);
  if (((int32_t)(pkt.timestamp - p->latest_packet_ts)) > 0)
    p->latest_packet_ts = pkt.timestamp;
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
    p->last_packet_arrival_time = ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
  queue_push(p->q, pkt);
  return 1;
}

int render_frame(player_t *p, int16_t *pcm) {
  if (!p || !pcm)
    return -1;
  process_input(p);
  if (p->warmup_frames > 0) {
    p->warmup_frames--;
    return idle(p, pcm);
  }
  player_stats_t s = get_player_stats(p);
  s = get_player_stats(p);
  int error = s.buffer_depth - p->target_depth;
  if (error > p->err_threshold) {
    skip_frames(p, error - 1);
    printf("skipping %d frames\n", error);
  } else if (error < -p->err_threshold) {
    if (s.gap > p->timeout) {
      printf("Silencio!\n");
      return idle(p, pcm);
    } else {
      printf("PLC outside playerstep\n");
      return plc(p, pcm);
    }
  }
  return player_step(p, pcm);
}

void player_destroy(player_t *p) {
  if (!p)
    return;

  opus_decoder_destroy(p->dec);
  destroy_buffer(p->jb);
  destroy_queue(p->q);
  free(p);
}
