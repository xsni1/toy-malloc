#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BLOCK_META_SIZE sizeof(struct block_meta)
#define ALIGNMENT 8

struct block_meta {
  size_t size;
  struct block_meta *next;
  struct block_meta *prev;
  int free;
};

void *HEAD = NULL;

struct block_meta *request_space(int n) {
  struct block_meta *cur = sbrk(0);
  struct block_meta *allocated = sbrk(n + BLOCK_META_SIZE);

  if (allocated == (void *)-1) {
    return NULL;
  }

  cur->next = NULL;
  cur->prev = NULL;
  cur->free = 0;
  cur->size = n;
  return cur;
}

/*
 *  ______________
 * |             |
 * |  next=0x00  |
 * |  prev=0x01  |
 * |  free=0     |
 * |  size=0     |
 * |_____________|
 *
 *
 * |_______________________________|
 *          0x0000 - 0x0020
 */

void *find_block(int n) {
  struct block_meta *cur = HEAD;

  while (cur->next) {
    if (cur->free == 1 && cur->size > n) {
      return cur;
    }

    cur = cur->next;
  }

  return cur;
}

void *append_or_reuse_block(struct block_meta *block, int n) {
  if (block->free && block->size > n) {
    if (block->size - n > BLOCK_META_SIZE + 8) {
      /* printf("moving by %lu\n", */
      /*        ((unsigned long)(char *)block + n + BLOCK_META_SIZE) % 8); */
      struct block_meta *new_block =
          (struct block_meta *)((char *)block + n + BLOCK_META_SIZE);
      new_block->size = block->size - n - BLOCK_META_SIZE;
      new_block->free = 1;
      new_block->next = block->next;
      block->next = new_block;
      new_block->prev = block;
      if (new_block->next != NULL) {
        new_block->next->prev = new_block;
      }
    }

    block->free = 0;
    block->size = n;
    return block;
  }

  struct block_meta *new_block = request_space(n);
  block->next = new_block;
  new_block->prev = block;

  return new_block;
}

// for n = 3, alignment = 8
// 3 + (3 - (3 % 8)) = 3 + (8 - (3 % 8))
int align(int n) { return n + (ALIGNMENT - (n % ALIGNMENT)); }

void tmalloc_print() {
  struct block_meta *cur = HEAD;
  while (cur) {
    printf("[address=%p size=%zu(%zu) free=%d next=%p prev=%p]\n", cur,
           cur->size, cur->size + BLOCK_META_SIZE, cur->free, cur->next,
           cur->prev);
    cur = cur->next;
  }
}

void tfree(struct block_meta *p) {
  if (!p) {
    return;
  }

  struct block_meta *block = ((struct block_meta *)p - 1);

  if (block->free) {
    return;
  }

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

void *tmalloc(int n) {
  int aligned_n = align(n);
  if (HEAD == NULL) {
    struct block_meta *cur = request_space(aligned_n);
    HEAD = cur;

    return cur + 1;
  }

  struct block_meta *block = find_block(aligned_n);
  struct block_meta *new_block = append_or_reuse_block(block, aligned_n);

  return new_block + 1;
}

int main() {
  void *addr = tmalloc(500);

  void *addr13 = tmalloc(1001);
  void *addr11 = tmalloc(1015);
  void *addr12 = tmalloc(1000);
  void *addr2 = tmalloc(1000);
  void *addr10 = tmalloc(1000);
  tfree(addr2);

  tmalloc_print();
  printf("======\n");

  void *addr14 = tmalloc(500);
  void *addr3 = tmalloc(800);
  void *addr5 = tmalloc(800);
  void *addr4 = tmalloc(800);

  tmalloc_print();
  printf("======\n");

  tfree(addr3);
  tfree(addr4);

  tmalloc_print();
  printf("======\n");

  tfree(addr5);

  tmalloc_print();
  printf("======\n");

  tfree(addr10);

  tmalloc_print();
  printf("======\n");

  tfree(addr14);
  tfree(addr13);
  tfree(addr11);
  tfree(addr12);

  tmalloc_print();
  printf("======\n");

  tfree(addr);

  tmalloc_print();
  printf("======\n");

  tmalloc(123);
  tmalloc_print();

  return 0;
}
