#ifndef _JB_C
#define _JB_C

#include "rtp.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct jitter_buffer {
  rtp_packet_t *packets;
  bool *valid;
  size_t capacity;
  size_t header;
};
#endif
