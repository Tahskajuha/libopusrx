#ifndef _PLAYER_H
#define _PLAYER_H

#include <opus.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct player {
  struct jitter_buffer *jb;
  struct queue *q;
  OpusDecoder *dec;

  int frame_size;
  int32_t window;
  int channels;
  int sample_rate;

  uint32_t current;
  uint16_t exp_seq;
  uint32_t latest_packet_ts;
  uint64_t last_packet_arrival_time;
  bool initialized;

  int buffer_depth;

  int target_depth;
  uint32_t timeout;
  int err_threshold;

  int warmup_frames;
} player_t;

#endif
