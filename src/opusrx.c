#define _POSIX_C_SOURCE 200809L

#include "opusrx/opusrx.h"
#include "pipeline.h"
#include "player.h"
#include <opus.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
  p->frame_size = cfg.frame_size;
  return p;
}

int ingest_rtp(const uint8_t *buffer, size_t len, player_t *p) {
  if (!buffer || !p)
    return -1;
  rtp_packet_t pkt;
  if (rtp_parse(buffer, len, &pkt) != 0)
    return 0;
  queue_push(p->q, pkt);
  return 1;
}

int process_input(player_t *p) {
  if (!p)
    return -1;
  rtp_packet_t pkt;
  // int32_t window = (p->jb->capacity - 1) * p->frame_size;

  int processed = 0;
  while (queue_pop(p->q, &pkt)) {
    if (!p->initialized) {
      p->current = pkt.timestamp;
      p->exp_seq = pkt.sequence_number;
      p->initialized = true;
      buffer_push(p->jb, pkt);
      processed++;
      continue;
    }
    // int32_t diff = (int32_t)((uint32_t)(pkt.timestamp - p->current));
    // if (diff > 0 && diff <= window) {
    buffer_push(p->jb, pkt);
    processed++;
    // }
  }
  return processed;
}

int player_step(player_t *p, int16_t *pcm) {
  if (!p || !pcm)
    return -1;
  rtp_packet_t pkt;
  int samples;
  if (buffer_get(p->jb, &pkt, p->exp_seq, true)) {
    samples = opus_decode(p->dec, pkt.payload, pkt.payload_len, pcm,
                          p->frame_size, 0);
  } else if (buffer_get(p->jb, &pkt, p->exp_seq + 1, false)) {
    samples = opus_decode(p->dec, pkt.payload, pkt.payload_len, pcm,
                          p->frame_size, 1);
    printf("FEC");
  } else {
    samples = opus_decode(p->dec, NULL, 0, pcm, p->frame_size, 0);
    printf("PLC");
  }
  p->exp_seq = (p->exp_seq + 1) & 0xFFFF;
  p->current += p->frame_size;
  return samples;
}

void player_destroy(player_t *p) {
  if (!p)
    return;

  opus_decoder_destroy(p->dec);
  destroy_buffer(p->jb);
  destroy_queue(p->q);
  free(p);
}
