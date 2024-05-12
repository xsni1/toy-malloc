#include <stdio.h>
#include <unistd.h>

#define BLOCK_META_SIZE sizeof(struct block_meta)

struct block_meta {
  size_t size;
  struct block_meta *next;
  int free;
};

void *HEAD = NULL;

void *request_space(int n) {
  struct block_meta *cur = sbrk(0);
  void *allocated = sbrk(n + BLOCK_META_SIZE);

  if (allocated == (void *)-1) {
    return NULL;
  }

  cur->next = NULL;
  cur->free = 0;
  cur->size = n;
  return cur;
}

void *tmalloc(int n) {
  if (HEAD == NULL) {
    void *cur = request_space(n);
    HEAD = cur;

    return cur + 1;
  }

  // TODO: look for already free space
  struct block_meta *cur = HEAD;
  while (cur->next) {
    cur = cur->next;
  }
  void *new_block = request_space(n);
  cur->next = new_block;

  return cur + 1;
}

void tmalloc_print() {
  struct block_meta *cur = HEAD;
  while (cur) {
    printf("[address=%p size=%zu free=%d]\n", cur, cur->size, cur->free);
    cur = cur->next;
  }
}

int main() {
  void *addr = tmalloc(1);
  while (1) {
    getchar();
    void *addr = tmalloc(1000);
    tmalloc_print();
    getchar();
  }
  return 0;
}
