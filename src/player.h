#ifndef _PLAYER_H
#define _PLAYER_H

#include <opus.h>
#include <stdint.h>

typedef struct player {
  struct jitter_buffer *jb;
  struct queue *q;
  OpusDecoder *dec;
  uint32_t current;
  uint16_t exp_seq;
  int frame_size;
} player_t;

#endif
