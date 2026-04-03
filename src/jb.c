#define _POSIX_C_SOURCE 200809L

#include "jb.h"
#include "rtp.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

struct jitter_buffer *init_buffer(size_t capacity) {
  if (capacity == 0)
    return NULL;
  struct jitter_buffer *jb = calloc(1, sizeof(struct jitter_buffer));
  if (!jb) {
    return NULL;
  }
  jb->packets = calloc(capacity, sizeof(rtp_packet_t));
  if (!jb->packets) {
    free(jb);
    return NULL;
  }
  jb->valid = calloc(capacity, sizeof(bool));
  if (!jb->valid) {
    free(jb->packets);
    free(jb);
    return NULL;
  }
  jb->capacity = capacity;
  jb->header = 0;
  return jb;
}

void buffer_push(struct jitter_buffer *jb, rtp_packet_t packet) {
  if (!jb)
    return;
  size_t idx = packet.sequence_number % jb->capacity;
  jb->packets[idx] = packet;
  jb->valid[idx] = true;
}

bool buffer_pop(struct jitter_buffer *jb, rtp_packet_t *out) {
  if (!jb || !out)
    return false;
  size_t idx = jb->header;
  jb->header = (jb->header + 1) % jb->capacity;
  bool is_valid = jb->valid[idx];

  if (is_valid) {
    *out = jb->packets[idx];
    jb->valid[idx] = false;
  }
  return is_valid;
}

void destroy_buffer(struct jitter_buffer *jb) {
  if (!jb)
    return;
  free(jb->packets);
  free(jb->valid);
  free(jb);
}
