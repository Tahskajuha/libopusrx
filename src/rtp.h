#ifndef _RTP_H
#define _RTP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
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

int rtp_parse(const uint8_t *buffer, size_t len, rtp_packet_t *out);

#endif
