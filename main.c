#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BLOCK_META_SIZE sizeof(struct block_meta)

struct block_meta {
  size_t size;
  struct block_meta *next;
  struct block_meta *prev;
  int free;
};

void *HEAD = NULL;

struct block_meta *request_space(int n) {
  struct block_meta *cur = sbrk(0);
  void *allocated = sbrk(n + BLOCK_META_SIZE);

  if (allocated == (void *)-1) {
    return NULL;
  }

  cur->next = NULL;
  cur->prev = NULL;
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

  while (cur->next) {
    // if free and big enough we gonna use this block to either take it as a
    // whole or split (if enough space)
    if (cur->free == 1 && cur->size > n) {
      // make 8 the smalles block of memory to fix alignment problems?

      // check if after taking n bytes of memory (as requested) from this
      // block, we can still use the leftovers for another block
      if (cur->size - n > BLOCK_META_SIZE + 8) {
        struct block_meta *new_block =
            (struct block_meta *)((char *)cur + n + BLOCK_META_SIZE);
        new_block->size = cur->size - n - BLOCK_META_SIZE;
        new_block->free = 1;
        new_block->next = cur->next;
        cur->next = new_block;
        new_block->prev = cur;
        new_block->next->prev = new_block;
      }

      cur->free = 0;
      cur->size = n;
      return cur + 1;
    }

    cur = cur->next;
  }

  struct block_meta *new_block = request_space(n);
  cur->next = new_block;
  new_block->prev = cur;

  return new_block + 1;
}

void tfree(struct block_meta *p) {
  if (!p) {
    return;
  }

  struct block_meta *block = ((struct block_meta *)p - 1);

  block->free = 1;
  if (block->next != NULL && block->next->free) {
    int old_block_size = block->next->size;
    if (block->next->next == NULL) {
      block->next = NULL;
    } else {
      block->next = block->next->next;
    }
    block->size = block->size + old_block_size + BLOCK_META_SIZE;
  }

  if (block->prev != NULL && block->prev->free) {
    int old_block_size = block->prev->size;
    if (block->prev->prev == NULL) {
      block->prev = NULL;
    } else {
      block->prev->prev->next = block;
      block->prev = block->prev->prev;
    }
    block->size = block->size + old_block_size + BLOCK_META_SIZE;
  }
}

void tmalloc_print() {
  struct block_meta *cur = HEAD;
  while (cur) {
    printf("[address=%p size=%zu(%zu) free=%d next=%p prev=%p]\n", cur, cur->size,
           cur->size + BLOCK_META_SIZE, cur->free, cur->next, cur->prev);
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
    void *addr10 = tmalloc(1000);
    tfree(addr2);
    tmalloc_print();

    printf("======\n");
    addr = tmalloc(500);
    void *addr3 = tmalloc(800);
    void *addr5 = tmalloc(800);
    void *addr4 = tmalloc(800);
    tfree(addr3);
    tfree(addr4);

    tmalloc_print();
    printf("======\n");

    tfree(addr5);
    tmalloc_print();
    printf("======\n");

    tfree(addr10);
    tmalloc_print();
    getchar();
  }
  return 0;
}
