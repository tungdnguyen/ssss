/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
Add a comment to this line
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define BLK_HDR_SIZE ALIGN(sizeof(blkhdr))
#define BLK_FTR_SIZE ALIGN(sizeof(blkftr))

typedef struct header blkhdr;
typedef struct footer blkftr;

struct header {
  size_t size;
  blkhdr *next_p;
  blkhdr *prev_p;
};

struct footer {
  blkhdr *hdr_ptr;
};
struct header hdr_list[14];

void *find_fit(size_t size);
int list_indexing(size_t size);


void *find_fit(size_t size)
{
  blkhdr *hp;
  int i;
  for (i = list_indexing(size); i < 14; i++) 
    {
      for (hp = hdr_list[i].next_p; (hp != &hdr_list[i]) && (hp->size < (size + SIZE_T_SIZE + BLK_FTR_SIZE)); hp=hp->next_p);
      if (hp != &hdr_list[i]) 
	{
	  return hp;
	}
    }
  return NULL;
}


int list_indexing(size_t size) 
{
  int alloc_size[14] = {16, 64, 80, 112, 128, 160, 448, 464, 512, 1620, 4072, 4095, 8190, 99999999};
  int idx;
  for (idx = 0; idx < 14; idx++) 
    {
      if (size < alloc_size[idx]+1) 
	{
	  return idx;
	}
    }
  return idx;
}

/*
 * mm_init - initialize the malloc package.
 */

int free_blocks;

int mm_init(void)
{
  free_blocks = 0;
  blkhdr *hp = mem_sbrk(BLK_HDR_SIZE + BLK_FTR_SIZE);
  hp->size = BLK_HDR_SIZE + BLK_FTR_SIZE + 1;
  hp->next_p = hp;
  hp->prev_p = hp;
  blkftr *fp = (blkftr *)((char *)hp + BLK_HDR_SIZE);
  fp->hdr_ptr = hp;
  int i;
  for (i = 0; i < 14; i++) 
    {
      hdr_list[i].size = BLK_HDR_SIZE + BLK_FTR_SIZE + 1;
      hdr_list[i].prev_p = &hdr_list[i];
      hdr_list[i].next_p = &hdr_list[i];
    }

  return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
  size_t newsize = ALIGN(size + SIZE_T_SIZE + BLK_FTR_SIZE);
  blkhdr *hp = (blkhdr *)find_fit(newsize);
  if (hp) 
    {
      if (free_blocks > 0) 
	{
	  if (((hp->size)&~1) - newsize >= 0x7 + BLK_HDR_SIZE + BLK_FTR_SIZE) 
	    {
	      size_t prev_size = hp->size;
	      hp->size = newsize;
	      ((blkftr *)((char *)hp + ((hp->size)&~1) - BLK_FTR_SIZE))->hdr_ptr = hp;
	      blkhdr *split = (blkhdr *)((char *)hp + (hp->size & ~1));
	      split->size = prev_size - hp->size;
	      ((blkftr *)((char *)split + ((split->size)&~1) - BLK_FTR_SIZE))->hdr_ptr = split;
	      int idx = list_indexing(split->size - SIZE_T_SIZE - BLK_FTR_SIZE);
	      split->next_p = hdr_list[idx].next_p;
	      hdr_list[idx].next_p = split;
	      split->next_p->prev_p = split;
	      split->prev_p = &hdr_list[idx];
	      free_blocks++;
	    }
      
	  if (free_blocks > 0) 
	    {
	      free_blocks--;
	    }
      
	  hp->prev_p->next_p = hp->next_p;
	  hp->next_p->prev_p = hp->prev_p;
	  hp->size |= 1;
	}
    }
  else 
    {
      blkhdr *bottom = (((blkftr *)((char *)mem_heap_hi() + 1 - BLK_FTR_SIZE))->hdr_ptr);

      if (((bottom->size)&1) == 0) 
	{
	  if (free_blocks > 0) 
	    {
	      free_blocks--;
	      mem_sbrk(newsize - bottom->size);
	      bottom->size = newsize | 1;
	      ((blkftr *)((char *)bottom + ((bottom->size)&~1) - BLK_FTR_SIZE))->hdr_ptr = bottom;
	      bottom->prev_p->next_p = bottom->next_p;
	      bottom->next_p->prev_p = bottom->prev_p;
	      hp = bottom;
	    }
	}
    
      else 
	{
	  hp = mem_sbrk(newsize);
	  if ((long)hp == -1) 
	    {
	      return NULL;
	    }
	  else 
	    {
	      hp->size = newsize | 1;
	      blkftr *fp = (blkftr *)((char *)hp + ((hp->size) & ~1) - BLK_FTR_SIZE);
	      fp->hdr_ptr = hp;
	    }
	}
    }
  
  return (void *)((char *)hp + sizeof(size_t));
}

/*
 * mm_free - Mark block as free and try to coalesce free blocks if possible
 */


void mm_free(void *ptr)
{
  free_blocks++;
  blkhdr *hp = (blkhdr *)((char *)ptr - SIZE_T_SIZE);
  blkhdr *hp_prev = ((blkftr *)((char *)hp - BLK_FTR_SIZE))->hdr_ptr;
  hp->size &= ~1;
  
  if (((hp_prev->size)&1) == 0) 
    {
      hp_prev->prev_p->next_p = hp_prev->next_p;
      hp_prev->next_p->prev_p = hp_prev->prev_p;
      hp_prev->size += hp->size;
      ((blkftr *)((char *)hp + ((hp->size)&~1) - BLK_FTR_SIZE))->hdr_ptr = hp_prev;
      hp = hp_prev;
      free_blocks--;
    }

  blkhdr *hp_next = (blkhdr *)((char *)hp + (hp->size &= ~1));
  
  if (((hp_next->size)&1) == 0) 
    {
      if (((void *)((char *)hp + hp->size) <= mem_heap_hi())) 
	{
	  hp->size += hp_next->size;
	  ((blkftr *)((char *)hp + ((hp->size)&~1) - BLK_FTR_SIZE))->hdr_ptr = hp;
	  hp_next->prev_p->next_p = hp_next->next_p;
	  hp_next->next_p->prev_p = hp_next->prev_p;
	  free_blocks--;
	}
    }

  blkhdr *top = &hdr_list[list_indexing(hp->size - SIZE_T_SIZE - BLK_FTR_SIZE)];
  hp->prev_p = top;
  hp->next_p = top->next_p;
  top->next_p->prev_p = hp;
  top->next_p = hp;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
  size_t newsize = ALIGN(size + SIZE_T_SIZE + BLK_FTR_SIZE);
  blkhdr *hp = (blkhdr *)((char *)ptr - sizeof(size_t));
  blkhdr *hp_next = (blkhdr *)((char *)hp + (hp->size &= ~1));

  if ((hp->size&~1) > newsize) 
    {
      return ptr;
    }

  if (((void *)((char *)hp + hp->size) >= mem_heap_hi())) 
    {
      mem_sbrk(newsize - hp->size);
      hp->size = newsize;
      ((blkftr *)((char *)hp + (hp->size&~1) - BLK_FTR_SIZE))->hdr_ptr = hp;
      return ptr;
    }

  if (((hp_next->size)&1) == 0) 
    {
      if (hp_next->size + (hp->size&~1) > newsize) 
	{
	  if (((void *)((char *)hp + hp->size) <= mem_heap_hi())) 
	    {
	      hp->size = (hp->size + hp_next->size) | 1;
	      ((blkftr *)((char *)hp + ((hp->size)&~1) - BLK_FTR_SIZE))->hdr_ptr = hp;
	      hp_next->prev_p->next_p = hp_next->next_p;
	      hp_next->next_p->prev_p = hp_next->prev_p;
	      return ptr;
	    }
	}
    }

  void *new_ptr = mm_malloc(size);
  memcpy(new_ptr,ptr,hp->size - SIZE_T_SIZE - BLK_FTR_SIZE);
  mm_free(ptr);
  return new_ptr;
}

