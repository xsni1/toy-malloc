#include <stdio.h>
#include <stdlib.h>
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

  struct block_meta *cur = HEAD;

  // first-fit algorithm
  // TODO: if free block is bigger than requested split it (new block has to be
  // big enough to contain meta block)
  while (cur->next) {
    // if free and big enough we gonna use this block to either take it as a
    // whole or split (if enough space)
    if (cur->free == 1 && cur->size > n) {
      // make 8 the smalles block of memory to fix alignment problems?

      // check if after taking n bytes of memory (as requested) from this
      // block, we can still use the leftovers for another block
      if (cur->size - n > BLOCK_META_SIZE + 8) {
        struct block_meta *new_block = (struct block_meta *)((char *)cur + n + BLOCK_META_SIZE);
        new_block->size = cur->size - n - BLOCK_META_SIZE;
        new_block->free = 1;
        new_block->next = cur->next;
        cur->next = new_block;
      }

      cur->free = 0;
      cur->size = n;
      return cur + 1;
    }

    cur = cur->next;
  }

  void *new_block = request_space(n);
  cur->next = new_block;

  return cur + 1;
}

void tfree(struct block_meta *p) {
  if (!p) {
    return;
  }

  // what happens if i try to change "free" field on some arbitrary address?
  ((struct block_meta *)p - 1)->free = 1;
}

void tmalloc_print() {
  struct block_meta *cur = HEAD;
  while (cur) {
    printf("[address=%p size=%zu(%zu) free=%d]\n", cur, cur->size,
           cur->size + BLOCK_META_SIZE, cur->free);
    cur = cur->next;
  }
}

int main() {
  void *addr = tmalloc(1);

  while (1) {
    getchar();
    void *addr = tmalloc(1000);
    addr = tmalloc(1000);
    addr = tmalloc(1000);
    void *addr2 = tmalloc(1000);
    addr = tmalloc(1000);
    tfree(addr2);
    tmalloc_print();
    printf("======\n");
    addr = tmalloc(500);

    tmalloc_print();
    getchar();
  }
  return 0;
}
