#ifndef _JB_C
#define _JB_C

#include "opusrx/opusrx.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct jitter_buffer {
  rtp_packet_t *packets;
  bool *valid;
  size_t capacity;
};

struct queue {
  rtp_packet_t *packets;
  size_t capacity;
  size_t front;
  size_t size;
};

struct jitter_buffer *init_buffer(size_t capacity);
struct queue *init_queue(size_t capacity);
void destroy_buffer(struct jitter_buffer *jb);
void destroy_queue(struct queue *q);
void queue_push(struct queue *q, rtp_packet_t packet);
bool queue_pop(struct queue *q, rtp_packet_t *out);
void buffer_push(struct jitter_buffer *jb, rtp_packet_t packet);
bool buffer_get(struct jitter_buffer *jb, rtp_packet_t *out, uint16_t seq,
                bool consume);

#endif
