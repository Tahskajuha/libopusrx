#define _POSIX_C_SOURCE 200809L

#include "pipeline.h"
#include "opusrx/primitives.h"
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
  // pthread_mutex_init(&q->lock, NULL);
  return q;
}

void buffer_push(struct jitter_buffer *jb, rtp_packet_t packet) {
  if (!jb)
    return;
  size_t idx = packet.sequence_number % jb->capacity;
  jb->packets[idx] = packet;
  jb->valid[idx] = true;
  jb->size++;
}

void queue_push(struct queue *q, rtp_packet_t packet) {
  if (!q)
    return;
  // pthread_mutex_lock(&q->lock);
  size_t idx = (q->front + q->size) % q->capacity;
  q->packets[idx] = packet;
  if (q->size == q->capacity) {
    q->front = (q->front + 1) % q->capacity;
  } else {
    q->size++;
  }
  // pthread_mutex_unlock(&q->lock);
}

bool buffer_get(struct jitter_buffer *jb, rtp_packet_t *out, uint16_t seq,
                bool consume) {
  if (!jb || !out)
    return false;
  size_t idx = seq % jb->capacity;
  bool is_valid = jb->valid[idx];

  if (is_valid && jb->packets[idx].sequence_number == seq) {
    *out = jb->packets[idx];
    jb->valid[idx] = consume ? false : jb->valid[idx];
  } else {
    is_valid = false;
  }
  if (consume)
    jb->size--;
  return is_valid;
}

bool queue_pop(struct queue *q, rtp_packet_t *out) {
  if (!q || !out)
    return false;

  // pthread_mutex_lock(&q->lock);
  if (q->size == 0)
    return false;

  *out = q->packets[q->front];
  q->front = (q->front + 1) % q->capacity;
  q->size--;
  // pthread_mutex_unlock(&q->lock);

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
  // pthread_mutex_destroy(&q->lock);
  free(q->packets);
  free(q);
}
