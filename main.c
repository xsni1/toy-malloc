#include <stdio.h>
#include <unistd.h>

struct block_meta {
  size_t size;
  struct block_meta *next;
  int free;
};

void *HEAD = NULL;

void *tmalloc(int n) {
  if (HEAD == NULL) {
    struct block_meta *cur = sbrk(0);
    void *allocated = sbrk(n);

    if (allocated == (void *)-1) {
      return NULL;
    }

    cur->next = NULL;
    cur->free = 0;
    cur->size = n;
    HEAD = cur;

    return cur + 1;
  }

  return NULL;
}

int main() {
  void *addr = tmalloc(1);
  /* printf("block_meta size=%lu. addr: %p\n", sizeof(struct block_meta), addr); */
  while (1) {}
  return 0;
}
