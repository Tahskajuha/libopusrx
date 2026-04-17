#define _POSIX_C_SOURCE 200809L

#include "opusrx/opusrx.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

int rtp_parse(const uint8_t *buffer, size_t len, rtp_packet_t *out) {
  if (!buffer || !out || len < 12)
    return -1;

  // Initialize pad_len, ext_len to zero to avoid branching when parsing payload
  uint8_t pad_len = 0;
  uint16_t ext_len = 0;

  out->version = (buffer[0] >> 6) & 0x03;
  if (out->version != 2)
    return -2;
  out->padding = (buffer[0] >> 5) & 0x01;
  out->extension = (buffer[0] >> 4) & 0x01;
  out->csrc_count = buffer[0] & 0x0F;

  size_t header_len = 12 + (4 * out->csrc_count);
  if (len < header_len)
    return -1;

  if (out->padding) {
    pad_len = buffer[len - 1];
  }
  if (out->extension) {
    // If extension exists then at least 4 additional bytes containing
    // profile-specific data and extension length must exist
    if (len < header_len + 4)
      return -1;

    // Length of the extension is at byte 2 and byte 3 of the extension
    uint16_t ext_word_amt = (uint16_t)buffer[header_len + 2] << 8 |
                            (uint16_t)buffer[header_len + 3];

    // Extension length does not include the 4 additional bytes; add them to get
    // total extension length
    ext_len = 4 + (4 * ext_word_amt);
  }

  if (len < header_len + ext_len + pad_len)
    return -1;

  out->marker = (buffer[1] >> 7) & 0x01;
  out->payload_type = buffer[1] & 0x7F;

  out->sequence_number = (uint16_t)buffer[2] << 8 | (uint16_t)buffer[3];

  out->timestamp = ((uint32_t)buffer[4] << 24 | (uint32_t)buffer[5] << 16) |
                   ((uint32_t)buffer[6] << 8 | (uint32_t)buffer[7]);

  out->ssrc = ((uint32_t)buffer[8] << 24 | (uint32_t)buffer[9] << 16) |
              ((uint32_t)buffer[10] << 8 | (uint32_t)buffer[11]);

  for (uint8_t i = 0; i < out->csrc_count; i++) {
    size_t offset = 4 * i;
    out->csrc[i] =
        ((uint32_t)buffer[12 + offset] << 24 | (uint32_t)buffer[13 + offset]
                                                   << 16) |
        ((uint32_t)buffer[14 + offset] << 8 | (uint32_t)buffer[15 + offset]);
  }

  out->payload = buffer + header_len + ext_len;
  out->payload_len = len - (header_len + ext_len + pad_len);

  return 0;
}
