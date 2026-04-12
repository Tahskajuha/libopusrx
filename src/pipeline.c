#define _POSIX_C_SOURCE 200809L

#include "pipeline.h"
#include "player.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

struct jitter_buffer *init_buffer(size_t capacity) {
  if (capacity == 0)
    return NULL;
  struct jitter_buffer *jb = calloc(1, sizeof(struct jitter_buffer));
  if (!jb)
    return NULL;
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

struct queue *init_queue(size_t capacity) {
  if (capacity == 0)
    return NULL;
  struct queue *q = calloc(1, sizeof(struct queue));
  if (!q)
    return NULL;
  q->packets = calloc(capacity, sizeof(rtp_packet_t));
  if (!q->packets) {
    free(q);
    return NULL;
  }
  q->capacity = capacity;
  q->front = 0;
  q->size = 0;
  return q;
}

void buffer_push(struct jitter_buffer *jb, rtp_packet_t packet) {
  if (!jb)
    return;
  size_t idx = packet.sequence_number % jb->capacity;
  jb->packets[idx] = packet;
  jb->valid[idx] = true;
}

void queue_push(struct queue *q, rtp_packet_t packet) {
  if (!q)
    return;
  size_t idx = (q->front + q->size) % q->capacity;
  q->packets[idx] = packet;
  if (q->size == q->capacity) {
    q->front = (q->front + 1) % q->capacity;
  } else {
    q->size++;
  }
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

bool buffer_peek(struct jitter_buffer *jb, rtp_packet_t *out) {
  if (!jb || !out) {
    return false;
  }
  size_t idx = jb->header;
  bool is_valid = jb->valid[idx];

  if (is_valid) {
    *out = jb->packets[idx];
  }
  return is_valid;
}

bool queue_pop(struct queue *q, rtp_packet_t *out) {
  if (q->size == 0)
    return false;

  *out = q->packets[q->front];
  q->front = (q->front + 1) % q->capacity;
  q->size--;

  return true;
}

void destroy_buffer(struct jitter_buffer *jb) {
  if (!jb)
    return;
  free(jb->packets);
  free(jb->valid);
  free(jb);
}

void destroy_queue(struct queue *q) {
  if (!q)
    return;
  free(q->packets);
  free(q);
}

int process_input_queue(player_t *p) {
  if (!p)
    return -1;
  rtp_packet_t pkt;
  uint32_t window = p->jb->capacity * p->frame_size;

  int32_t lower = -(int32_t)((window * 4) / 10);
  int32_t upper = (int32_t)((window * 6) / 10);

  int processed = 0;
  while (queue_pop(p->q, &pkt)) {
    int32_t diff = (int32_t)((uint32_t)(pkt.timestamp - p->current));
    if (diff > lower && diff <= upper) {
      buffer_push(p->jb, pkt);
      processed++;
    }
  }
  return processed;
}
