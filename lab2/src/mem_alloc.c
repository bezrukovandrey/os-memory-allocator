#include "mem_alloc.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#include "mem_alloc_types.h"
#include "my_mmap.h"

#define MEM_POOL_SIZE 1024

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MIN_BLOCK_SIZE MAX(sizeof(memblk_allocated_t), sizeof(memblk_free_t)) - MIN(sizeof(memblk_allocated_t), sizeof(memblk_free_t))



#if defined(NEXT_FIT)
memblk_free_t *pNext_fit = NULL; 
#endif


void *heap_begin;

memblk_free_t *first_free; 

#define ULONG(x)((long unsigned int)(x))

#if defined(FIRST_FIT)

void * memory_alloc(size_t size)
{
    memblk_free_t *prev_free = NULL; 
    memblk_free_t *curr_free = first_free;
    if (size < MIN_BLOCK_SIZE) {
        size = MIN_BLOCK_SIZE;
    }

    while (curr_free) {
        if (curr_free->size >= size) {
            if (curr_free->size >= size + MAX(sizeof(memblk_allocated_t), sizeof(memblk_free_t)) ) {
                memblk_free_t *new_free = (char *) curr_free + sizeof(memblk_allocated_t) + size;
                memblk_free_t* curr_next_pointer = curr_free->next_free; 

                new_free->size = curr_free->size - sizeof(memblk_allocated_t) - size; // free headers are mutualy deducted
                new_free->next_free = curr_next_pointer;
                if (prev_free) {
                    prev_free->next_free = new_free;
                } else {
                    first_free = new_free;
                }   
                ((memblk_allocated_t*)curr_free)->size = size;
            } else {
                if (prev_free)
                    prev_free->next_free = curr_free->next_free;
                else
                    first_free = curr_free->next_free;
            }
            void *p_payload = (char *) curr_free + sizeof(memblk_allocated_t);
            print_alloc_info(p_payload, size);
            return p_payload;
        }
        prev_free = curr_free;
        curr_free = curr_free->next_free;
    }
    print_alloc_info(NULL, size); 
    exit(0);
    
}

#elif defined(BEST_FIT)

void *memory_alloc(size_t size)
{

    memblk_free_t *prev_free = NULL; 
    memblk_free_t *curr_free = first_free; 
    memblk_free_t *best_free = NULL;
    memblk_free_t *best_prev_free = NULL;

    size_t best_diff = MEM_POOL_SIZE ;
    if (size < MIN_BLOCK_SIZE) {
        size = MIN_BLOCK_SIZE;
    }

    while (curr_free) {
        if (curr_free->size >= size && curr_free->size  - size < best_diff) {
            best_free = curr_free; 
            best_diff = curr_free->size  - size;
            best_prev_free = prev_free;
        }
        prev_free = curr_free;
        curr_free = curr_free->next_free;
    }

    if(!best_free) {
        print_alloc_error(size);
        exit(0);
    }

    if ( best_free->size >= size + MAX(sizeof(memblk_allocated_t), sizeof(memblk_free_t)) ) {
                memblk_free_t *new_free = (char *)  best_free + sizeof(memblk_allocated_t) + size;
                memblk_free_t* curr_next_pointer = best_free->next_free;
                new_free->size =  best_free->size - sizeof(memblk_allocated_t) - size ;
                new_free->next_free = curr_next_pointer;

                if (best_prev_free) 
                    best_prev_free->next_free = new_free;
                else    
                    first_free = new_free;
                best_free->size = size;
            } else {
                if (best_prev_free)
                    best_prev_free->next_free = best_free->next_free;
                else
                    first_free = best_free->next_free;
            }
    void *p_payload = (char *) best_free + sizeof(memblk_allocated_t); 
    print_alloc_info(p_payload, size);
    return p_payload;
}

#elif defined(NEXT_FIT)

void *memory_alloc(size_t size)
{
    if (!first_free) {
        print_alloc_info(NULL, size);
        exit(0);
    }
    if (size < MIN_BLOCK_SIZE) {
        size = MIN_BLOCK_SIZE;
    }

    memblk_free_t *prev_free = NULL; 
    memblk_free_t *curr_free = pNext_fit;

    int is_loop_passed = 0;

    while (curr_free && (curr_free != pNext_fit || !is_loop_passed)){
        if (curr_free->size >= size) {
            if (curr_free->size >= size + MAX(sizeof(memblk_allocated_t), sizeof(memblk_free_t)) ) { 
                memblk_free_t *new_free = (memblk_free_t *)((char *) curr_free + sizeof(memblk_allocated_t) + size);
                memblk_free_t* curr_next_pointer = curr_free->next_free;
                new_free->size = curr_free->size - sizeof(memblk_allocated_t) - size ;
                new_free->next_free = curr_next_pointer;
                if (prev_free) 
                    prev_free->next_free = new_free;
                else    
                     first_free = new_free;
                curr_free->size = size;
            } else {
                if (prev_free)
                    prev_free->next_free = curr_free->next_free;
                else
                    first_free = curr_free->next_free;
            }
            pNext_fit = curr_free->next_free ? curr_free->next_free : first_free;
            void *p_payload = (char *) curr_free + sizeof(memblk_allocated_t);
            print_alloc_info(p_payload, size);
            return p_payload;
        }
        
        if (curr_free->next_free) {
            prev_free = curr_free;
            curr_free = curr_free->next_free;
        } else {
            curr_free = first_free;
            prev_free = NULL;
            is_loop_passed = 1;
        }
    }
    print_alloc_info(NULL, size);
    exit(0);
}

#endif

void run_at_exit(void)
{
}

void memory_init(void)
{
    /* register the function that will be called when the programs exits */
    atexit(run_at_exit);

    heap_begin = my_mmap(MEM_POOL_SIZE);

    if (!heap_begin) {
        fprintf(stderr, "Error: Couldn't allocate the memory pool");
        exit(1);
    }

    first_free = (memblk_free_t *) heap_begin;
    first_free->next_free = NULL;
    first_free->size = MEM_POOL_SIZE - sizeof(memblk_free_t);

    #if defined(NEXT_FIT)
       pNext_fit = first_free;
    #endif 
}


void memory_free(void *p) 
{

    if (!p) {
        fprintf(stderr, "Invalid pointer %p passed to free\n", p);
        exit(1);
    }
    
    
    if (p < heap_begin || p >= (char *)heap_begin + MEM_POOL_SIZE) {
        fprintf(stderr, "Invalid pointer %p passed to free\n", p);
    }
    memblk_allocated_t* allocated_block = (memblk_allocated_t *)(p - sizeof(memblk_allocated_t));

    memblk_free_t *block = (memblk_free_t *)(p - sizeof(memblk_allocated_t)); 
    memblk_free_t *curr_free = first_free;
    memblk_free_t *prev_free = NULL;

    while (curr_free && curr_free < block) {
        prev_free = curr_free;
        curr_free = curr_free->next_free;
    }

    block->next_free = curr_free;
    if (prev_free)
        prev_free->next_free = block;
    else
        first_free = block;

    if (curr_free && (char *)block + sizeof(memblk_allocated_t) + block->size == (char *)curr_free) {
        block->size += sizeof(memblk_allocated_t) + curr_free->size;
        block->next_free = curr_free->next_free;
        #if defined(NEXT_FIT)
        if ((char *) pNext_fit >= (char *) block && (char *) pNext_fit < (char *) block + sizeof(memblk_allocated_t) + block->size) {
            pNext_fit = block;
        } 
        #endif
    }

    if (prev_free && (char *)prev_free + sizeof(memblk_allocated_t) + prev_free->size == (char *)block) {
        prev_free->size += sizeof(memblk_allocated_t) + block->size;
        prev_free->next_free = block->next_free;
       #if defined(NEXT_FIT)
        if ((char *) pNext_fit == (char *) block) {
            pNext_fit = prev_free;
        } 
        #endif 
    }


    print_free_info(p);

}

size_t memory_get_allocated_block_size(void *addr)
{

    if (!addr)
        return 0;

    memblk_allocated_t *block = (memblk_allocated_t *)((char *)addr + sizeof(memblk_allocated_t)); // - or +, i suppose +, can't test
    return block->size;

}

void print_free_list(void) 
{
    
    memblk_free_t *curr_free = first_free;
    while (curr_free) {
    fprintf(stderr, "Block FREE start: %p (%d) size %d, end: %p\n", (char *) curr_free, (int) ((char *) curr_free - (char *) heap_begin), curr_free->size, curr_free->next_free);
        curr_free = curr_free->next_free;
    }
}

void print_mem_state(void) 
{
    print_free_list();
    fprintf(stderr, "Current memory state:\n");
    fprintf(stderr, "[");

    for (int i = 0; i < MEM_POOL_SIZE; i++){
        memblk_free_t *curr_free = first_free;
        int is_free = 0;

        while (curr_free) {
            size_t curr_left = (char *) curr_free - (char *) heap_begin;
            size_t curr_right = curr_left + sizeof(memblk_allocated_t) + curr_free->size;
            if (i >= curr_left && i < curr_right){
                is_free = 1;
                break;
            }
            curr_free = curr_free->next_free;
        }
        fprintf(stderr, is_free ? "-" : "+");
        if ((i + 1) % 64 == 0 && (i + 1) < MEM_POOL_SIZE) {
            fprintf(stderr, "]\n[");
        } else if ((i + 1) % 64 == 0 && (i < MEM_POOL_SIZE)) {
            fprintf(stderr, "]\n");
            };
        } 
    
}


void print_info(void) {
    fprintf(stderr, "Memory : [%lu %lu] (%lu bytes)\n", (long unsigned int) heap_begin, (long unsigned int) ((char*)heap_begin+MEM_POOL_SIZE), (long unsigned int) (MEM_POOL_SIZE));
}

void print_free_info(void *addr){
    if(addr){
        fprintf(stderr, "FREE  at : %lu \n", ULONG((char*)addr - (char*)heap_begin));
    }
    else{
        fprintf(stderr, "FREE  at : %lu \n", ULONG(0));
    }
    
}

void print_alloc_info(void *addr, int requested_size){
  if(addr){
    fprintf(stderr, "ALLOC at : %lu (%d byte(s))\n", 
	    ULONG((char*)addr - (char*)heap_begin), requested_size);
  }
  else{
    fprintf(stderr, "Warning, system is out of memory\n"); 
  }
}

void print_alloc_error(int requested_size) 
{
    fprintf(stderr, "ALLOC error : can't allocate %d bytes\n", requested_size);
}


#ifdef MAIN
int main(int argc, char **argv){

  /* The main can be changed, it is *not* involved in tests */
  memory_init();
  print_info();
  int i ; 
  for( i = 0; i < 10; i++){
    char *b = memory_alloc(rand()%8);
    memory_free(b);
  }

  char * a = memory_alloc(15);
  memory_free(a);


  a = memory_alloc(10);
  memory_free(a);

  fprintf(stderr,"%lu\n",(long unsigned int) (memory_alloc(9)));
  return EXIT_SUCCESS;
}
#endif 