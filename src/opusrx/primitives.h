#ifndef _PRIMITIVES_H
#define _PRIMITIVES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct player player_t;

typedef struct rtp_packet {
  uint8_t version;
  bool padding;
  bool extension;
  uint8_t csrc_count;
  bool marker;
  uint8_t payload_type;
  uint16_t sequence_number;
  uint32_t timestamp;
  uint32_t ssrc;

  uint32_t csrc[15];
  const uint8_t *payload;
  size_t payload_len;
} rtp_packet_t;

typedef struct {
  uint32_t current_ts;
  uint16_t next_expected_sequence;
  uint64_t gap;
  int32_t playout_lag;
  int buffer_depth;
  bool initialized;
} player_stats_t;

int process_input(player_t *p);
int player_step(player_t *p, int16_t *pcm);
int rtp_parse(const uint8_t *buffer, size_t len, rtp_packet_t *out);
player_stats_t get_player_stats(player_t *p);
int skip_frames(player_t *p, int error);
int idle(player_t *p, int16_t *pcm);
int plc(player_t *p, int16_t *pcm);
const uint8_t *payload_copy(const uint8_t *payload, size_t payload_len);

#endif
