#define _POSIX_C_SOURCE 200809L

#include "opusrx/primitives.h"
#include "pipeline.h"
#include "player.h"
#include <opus.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int process_input(player_t *p) {
  if (!p)
    return -1;
  rtp_packet_t pkt;

  int processed = 0;
  player_stats_t s = get_player_stats(p);
  while (queue_pop(p->q, &pkt)) {
    if (!p->initialized || s.playout_lag > p->window || s.playout_lag < 0 ||
        s.gap > p->timeout) {
      p->current = pkt.timestamp;
      p->exp_seq = pkt.sequence_number;
      p->initialized = true;
      buffer_push(p->jb, pkt);
      printf("Pushed: %d\n", pkt.sequence_number);
      processed++;
      p->warmup_frames = 10;
      continue;
    }
    int32_t diff = (int32_t)(pkt.timestamp - p->current);
    if (diff >= 0 && diff <= p->window) {
      buffer_push(p->jb, pkt);
      printf("Pushed: %d\n", pkt.sequence_number);
      processed++;
    } else {
      printf("Packet dropped\n");
    }
  }
  p->buffer_depth = p->jb->size;
  return processed;
}

int plc(player_t *p, int16_t *pcm) {
  return opus_decode(p->dec, NULL, 0, pcm, p->frame_size, 0);
}

int skip_frames(player_t *p, int error) {
  if (error <= 0 || !p)
    return -1;
  int16_t tmp[p->frame_size * p->channels];
  for (int i = 0; i < error; i++) {
    player_step(p, tmp);
  }
  return 0;
}

int idle(player_t *p, int16_t *pcm) {
  memset(pcm, 0, p->frame_size * p->channels * sizeof(int16_t));
  return p->frame_size;
}

int player_step(player_t *p, int16_t *pcm) {
  if (!p || !pcm)
    return -1;
  rtp_packet_t pkt;
  int samples;
  if (buffer_get(p->jb, &pkt, p->exp_seq, true)) {
    samples = opus_decode(p->dec, pkt.payload, pkt.payload_len, pcm,
                          p->frame_size, 0);
    free((void *)pkt.payload);
    printf("Found packet\n");
  } else if (buffer_get(p->jb, &pkt, p->exp_seq + 1, false)) {
    samples = opus_decode(p->dec, pkt.payload, pkt.payload_len, pcm,
                          p->frame_size, 1);
    printf("FEC\n");
  } else {
    samples = plc(p, pcm);
    printf("PLC inside playerstep\n");
  }
  p->exp_seq = (p->exp_seq + 1) & 0xFFFF;
  p->current += p->frame_size;
  printf("Next expected sequence: %d\n", p->exp_seq);
  return samples;
}

player_stats_t get_player_stats(player_t *p) {
  player_stats_t s = {0};
  s.current_ts = p->current;
  s.next_expected_sequence = p->exp_seq;
  s.buffer_depth = p->buffer_depth;
  s.initialized = p->initialized;

  struct timespec ts;
  uint64_t now_ms = p->last_packet_arrival_time;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
    now_ms = ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
  }
  s.gap = now_ms - p->last_packet_arrival_time;

  s.playout_lag = (int32_t)(p->latest_packet_ts - p->current);
  return s;
}
