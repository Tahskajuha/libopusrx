#define _POSIX_C_SOURCE 200809L

#include "opusrx.h"
#include "pipeline.h"
#include "player.h"
#include <opus.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

int ingest_rtp(const uint8_t *buffer, size_t len, player_t *p) {
  if (!buffer || !p)
    return -1;
  rtp_packet_t pkt;
  if (rtp_parse(buffer, len, &pkt) != 0)
    return 0;
  queue_push(p->q, pkt);
  return 1;
}

int player_step(player_t *p, float *pcm) {
  if (!p || !pcm)
    return -1;
  process_input_queue(p);
  rtp_packet_t pkt;
  int samples;
  if (buffer_pop(p->jb, &pkt) && pkt.sequence_number == p->exp_seq) {
    samples = opus_decode_float(p->dec, pkt.payload, pkt.payload_len, pcm,
                                p->frame_size, 0);
  } else if (buffer_peek(p->jb, &pkt) &&
             pkt.sequence_number == (p->exp_seq + 1)) {
    samples = opus_decode_float(p->dec, pkt.payload, pkt.payload_len, pcm,
                                p->frame_size, 1);
  } else {
    samples = opus_decode_float(p->dec, NULL, 0, pcm, p->frame_size, 0);
  }
  p->exp_seq = (p->exp_seq + 1) & 0xFFFF;
  p->current += p->frame_size;
  return samples;
}
