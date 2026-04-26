#ifndef _OPUSRX_H
#define _OPUSRX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct player player_t;

typedef struct {
  int frame_size;
  size_t buffer_size;
  size_t queue_size;

  int channels;
  uint32_t sample_rate;

  int target_depth;
  uint32_t timeout;
  int err_threshold;
} player_config_t;

player_t *init_player(const player_config_t cfg);
int render_frame(player_t *p, int16_t *pcm);
int ingest_rtp(const uint8_t *buffer, size_t len, player_t *p);
void player_destroy(player_t *p);

#endif
