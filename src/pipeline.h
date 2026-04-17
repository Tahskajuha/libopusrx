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
  size_t header;
};

struct queue {
  rtp_packet_t *packets;
  size_t capacity;
  size_t front;
  size_t size;
};

void queue_push(struct queue *q, rtp_packet_t packet);
void buffer_push(struct jitter_buffer *jb, rtp_packet_t packet);
bool buffer_pop(struct jitter_buffer *jb, rtp_packet_t *out);
bool buffer_peek(struct jitter_buffer *jb, rtp_packet_t *out);
bool queue_pop(struct queue *q, rtp_packet_t *out);
int process_input_queue(player_t *p);

#endif
