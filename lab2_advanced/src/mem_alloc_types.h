#ifndef   	_MEM_ALLOC_TYPES_H_
#define   	_MEM_ALLOC_TYPES_H_



/* Structure declaration for a free block */
struct memblk_free{
    size_t size;
    struct memblk_free_t *next_free;
}; 
typedef struct memblk_free memblk_free_t; 

/* Specific metadata for allocated blocks */
struct memblk_allocated{
    size_t size;
    int magic_number;
};
typedef struct memblk_allocated memblk_allocated_t;


#endif
