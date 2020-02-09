#include "mm.h"      // prototypes of functions implemented in this file

#include "memlib.h"  // mem_sbrk -- to extend the heap
#include <string.h>  // memcpy -- to copy regions of memory

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) > (y) ? (y) : (x))
#define SEGLIST_SIZE 24 //size of my segregated list array

/**
 * A block header uses 4 bytes for:
 * - a block size, multiple of 8 (so, the last 3 bits are always 0's)
 * - an allocated bit (stored as LSB, since the last 3 bits are needed)
 *
 * A block footer has the same format.
 * Check Figure 9.48(a) in the textbook.
 */
typedef int BlockHeader;

static int get_size(BlockHeader *bp) { //return blockHeader size
    return (*bp) & ~7;  // discard last 3 bits
}
static int get_allocated(BlockHeader *bp) { //alloc status
    return (*bp) & 1;   // get last bit
}

static void set_header(BlockHeader *bp, int size, int allocated) { //update size and allocated status of header
    *bp = size | allocated;
}
static void set_footer(BlockHeader *bp, int size, int allocated) { //same for footer
    char *footer_addr = (char *)bp + get_size(bp) - 4;
    // the footer has the same format as the header
    set_header((BlockHeader *)footer_addr, size, allocated);
}

static char *get_payload_addr(BlockHeader *bp) { // payload address is header+4
    return (char *)(bp + 1);
}
static BlockHeader *get_prev(BlockHeader *bp) { //find header of prev block on heap
    // move back by 4 bytes to find the footer of the previous block
    BlockHeader *previous_footer = bp - 1;
    int previous_size = get_size(previous_footer);
    char *previous_addr = (char *)bp - previous_size;
    return (BlockHeader *)previous_addr;
}
static BlockHeader *get_next(BlockHeader *bp) { //find header of next block on heap
    int this_size = get_size(bp);
    char *next_addr = (char *)bp + this_size;  // TODO: to implement, look at get_prev
    return (BlockHeader *)next_addr;
}

/**
 * In addition to the block header with size/allocated bit, a free block has
 * pointers to the headers of the previous and next blocks on the free list.
 *
 * Pointers use 4 bytes because this project is compiled with -m32.
 * Check Figure 9.48(b) in the textbook.
 */
typedef struct {
    BlockHeader header;
    BlockHeader *prev_free;
    BlockHeader *next_free;
} FreeBlockHeader;

static BlockHeader *get_prev_free(BlockHeader *bp) { //header address of prev free block
    FreeBlockHeader *fp = (FreeBlockHeader *)bp;
    return fp->prev_free;
}
static BlockHeader *get_next_free(BlockHeader *bp) { //header address of next free block
    FreeBlockHeader *fp = (FreeBlockHeader *)bp;
    return fp->next_free;
}
static void set_prev_free(BlockHeader *bp, BlockHeader *prev) { //set prev free block address
    FreeBlockHeader *fp = (FreeBlockHeader *)bp;
    fp->prev_free = prev;
}
static void set_next_free(BlockHeader *bp, BlockHeader *next) { //set next free block address
    FreeBlockHeader *fp = (FreeBlockHeader *)bp;
    fp->next_free = next;
}

/* Pointer to the header of the first block on the heap */
static BlockHeader *heap_blocks;
/* Pointers to the headers of the first and last blocks on the free list */
static BlockHeader *free_headp;
static BlockHeader *free_tailp;
/* Pointer to segregated free list */
BlockHeader *seg_free_list[SEGLIST_SIZE];

static void free_list_insert(BlockHeader *bp){ //insert freed block into segregate free list
    int list_count = 0;
    BlockHeader *search_ptr = NULL;
    BlockHeader *insert_ptr = NULL;
    int size = get_size(bp);
    int tempsize = size;
    //identify appropriate segregated list
    while( (list_count<(SEGLIST_SIZE-1)) && (tempsize>1) ){
        tempsize >>= 1;
        list_count++;
    }

    //segregated list points to the most recently put term and then you traverse backwards
    search_ptr = seg_free_list[list_count];
    while ((search_ptr != NULL) && (size > get_size(search_ptr)) ) { //sort the pointer by size in segregated list
        insert_ptr = search_ptr;
        search_ptr = get_next_free(search_ptr);
    }

    // Set predecessor and successor 
    if (search_ptr != NULL) {                   // storage position in the middle of the list
        if (insert_ptr != NULL) {
            set_next_free(bp, search_ptr);
            set_prev_free(search_ptr, bp);
            set_prev_free(bp, insert_ptr);
            set_next_free(insert_ptr, bp);
        } else {                                // append to the beginning of the list (list not empty)
            set_next_free(bp, search_ptr);
            set_prev_free(search_ptr, bp);
            set_prev_free(bp, NULL);
            seg_free_list[list_count] = bp;
        }
    } else {                                    // storage position at the end of the list
        if (insert_ptr != NULL) {
            set_next_free(bp, NULL);
            set_prev_free(bp, insert_ptr);
            set_next_free(insert_ptr, bp);
        } else {                                // list is empty
            set_prev_free(bp, NULL);
            set_next_free(bp, NULL);
            seg_free_list[list_count] = bp;
        }
    }
}
static void free_list_remove(BlockHeader *bp) { //remove a block from free list
    // TODO: implement
    if(get_allocated(bp)){
        return;
    }

    int list_count = 0;
    int size = get_size(bp);

    //identify appropriate segregated list
    while( (list_count<(SEGLIST_SIZE-1)) && (size>1) ){
        size = size >> 1;
        list_count++;
    }

    if (get_next_free(bp) != NULL) {                                // if bp has a successor term
        if (get_prev_free(bp) != NULL) {                            // check if bp is not the first term in seg list
            set_prev_free(get_next_free(bp), get_prev_free(bp));
            set_next_free(get_prev_free(bp), get_next_free(bp));
        } else {                                                    // bp is the first term and has successor
            set_prev_free(get_next_free(bp), NULL);
            seg_free_list[list_count] = get_next_free(bp);
        }
    } else {                                                        // bp doesn't have successor
        if (get_prev_free(bp) != NULL) {                                    // bp has predecessor
            set_next_free(get_prev_free(bp), NULL);
        } else {                                                    // bp has no predecessor (i.e. only 1 term in seg list)
            seg_free_list[list_count] = NULL;
        }
    }
}
static BlockHeader *free_coalesce(BlockHeader *bp) { // mark a block as free, coalesce with contiguous free block on heap, add coalesced block to free list

    // mark block as free
    int size = get_size(bp);
    set_header(bp, size, 0);
    set_footer(bp, size, 0);

    // check whether contiguous blocks are allocated
    int prev_alloc = get_allocated(get_prev(bp));
    int next_alloc = get_allocated(get_next(bp));

    if (prev_alloc && next_alloc) {                     //prev and next blocks allocated
        // TODO: add bp to free list
        return bp;

    } else if (prev_alloc && !next_alloc) {             //prev allocated, next free
        // TODO: remove next block from free list
        // TODO: add bp to free list        note: for a segregated free list, once size changes the free list may change
        // TODO: coalesce with next block         so insertion happens after coalescing
        free_list_remove(bp);
        free_list_remove(get_next(bp));
        size += get_size(get_next(bp));
        set_header(bp, size, 0);
        set_footer(bp, size, 0);
        free_list_insert(bp);
        return bp;

    } else if (!prev_alloc && next_alloc) {             //prev free, next allocated
        // TODO: coalesce with previous block
        free_list_remove(bp);
        free_list_remove(get_prev(bp));
        size += get_size(get_prev(bp));
        set_header(get_prev(bp), size, 0);
        set_footer(bp, size, 0);
        free_list_insert(get_prev(bp));
        return get_prev(bp);

    } else {                                            //prev and next blocks free
        // TODO: remove next block from free list
        // TODO: coalesce with previous and next block
        free_list_remove(bp);
        free_list_remove(get_prev(bp));
        free_list_remove(get_next(bp));
        size = size + get_size(get_prev(bp)) + get_size(get_next(bp));
        set_header(get_prev(bp), size, 0);
        set_footer(get_next(bp), size, 0);
        free_list_insert(get_prev(bp));
        return get_prev(bp);
    }
}

static BlockHeader *extend_heap(int size) { //extend heap with a free block of size bytes (multiple of 8)

    // bp points to the beginning of the new block
    char *bp = mem_sbrk(size);
    if ((long)bp == -1)
        return NULL;

    // write header over old epilogue, then the footer
    BlockHeader *old_epilogue = (BlockHeader *)bp - 1;
    set_header(old_epilogue, size, 0);
    set_footer(old_epilogue, size, 0);
    free_list_insert(old_epilogue);

    // write new epilogue
    set_header(get_next(old_epilogue), 0, 1);

    // merge new block with previous one if possible
    return free_coalesce(old_epilogue);
}
int mm_init(void) { //initialize malloc package called prior to malloc, realloc or free, used for initialization e.g. initial heap area

    // init list of free blocks
    free_headp = NULL;
    free_tailp = NULL;

    // Initialize segregated free list array
    for (int list = 0; list < SEGLIST_SIZE; list++) {
        seg_free_list[list] = NULL;
    }

    // Allocate memory for the initial empty heap
    // create empty heap of 4 x 4-byte words
    char *new_region = mem_sbrk(16);
    if ((long)new_region == -1)
        return -1;

    heap_blocks = (BlockHeader *)new_region;
    set_header(heap_blocks, 0, 0);      // skip 4 bytes for alignment
    set_header(heap_blocks + 1, 8, 1);  // allocate a block of 8 bytes as prologue
    set_footer(heap_blocks + 1, 8, 1);
    set_header(heap_blocks + 3, 0, 1);  // epilogue
    heap_blocks += 1;                   // point to the prologue header

    // TODO: extend heap with an initial heap size
    if (extend_heap(64) == NULL){ 
        return -1; }

    return 0;
}
void mm_free(void *bp) {
    // TODO: move back 4 bytes to find the block header, then free block
    BlockHeader *ptr = (BlockHeader *)bp - 1;
    int size = get_size(ptr);
    set_header(ptr, size, 0);
    set_footer(ptr, size, 0);
    free_list_insert(ptr);
    free_coalesce(ptr);
}

/**
 * Find a free block with size greater or equal to `size`.
 *
 * @param size minimum size of the free block
 * @return pointer to the header of a free block or `NULL` if free blocks are
 *         all smaller than `size`.
 */
static BlockHeader *find_fit(int size) {
    // TODO: implement
    BlockHeader *ptr = NULL;
    int searchsize = size;
    for(int i=0; i<SEGLIST_SIZE; i++){
        if( ( (searchsize <= 1) && (seg_free_list[i] != NULL) ) || (i == SEGLIST_SIZE-1) ){
            ptr = seg_free_list[i];
            while( (ptr!=NULL) && (size> get_size(ptr)) ){
                ptr = get_next_free(ptr);
            }
            if(ptr != NULL){
                break;
            }
        }
        searchsize >>= 1;
    }
    return ptr;
}
/**
 * Allocate a block of `size` bytes inside the given free block `bp`.
 *
 * @param bp pointer to the header of a free block of at least `size` bytes
 * @param size bytes to assign as an allocated block (multiple of 8)
 * @return pointer to the header of the allocated block
 */
static BlockHeader *place(BlockHeader *bp, int size) {
    // TODO: if current size is greater, use part and add rest to free list
    int bp_size = get_size(bp);
    int remainder = bp_size - size;
    
    free_list_remove(bp);

    if(remainder <= 16){ // 16 is the min block size, but we can adjust this count
        set_header(bp,bp_size,1);
        set_footer(bp,bp_size,1);
    } else { 
        if(remainder >= 100){
            set_header(bp,remainder,0);
            set_footer(bp,remainder,0);
            set_header(get_next(bp),size,1);
            set_footer(get_next(bp),size,1);
            free_list_insert(bp);
            return get_next(bp);
        }
        else{
            set_header(bp,size,1);
            set_footer(bp,size,1);
            set_header(get_next(bp),remainder,0);
            set_footer(get_next(bp),remainder,0);
            free_list_insert(get_next(bp));
        }
    }
    // TODO: return pointer to header of allocated block
    return bp;
}

/**
 * Compute the required block size (including space for header/footer) from the
 * requested payload size.
 *
 * @param payload_size requested payload size
 * @return a block size including header/footer that is a multiple of 8
 */
static int required_block_size(int payload_size) {
    payload_size += 8;                    // add 8 for for header/footer
    return ((payload_size + 7) / 8) * 8;  // round up to multiple of 8
}

void *mm_malloc(size_t size) {
    // ignore spurious requests
    if (size == 0)
        return NULL;

    int asize = 0;
    if (size <= 8){
        asize = 16;
    } else{
        asize = required_block_size(size);
    }

    // TODO: find a free block or extend heap
    BlockHeader *alloc = find_fit(asize);
    if(alloc != NULL){
        alloc = place(alloc, asize);
    } else{
        int extend_heap_size = MAX(asize, 4096);
        if ( (alloc = extend_heap(extend_heap_size)) == NULL){
            return NULL;
        }
        alloc = place(alloc, asize);
    }
    return get_payload_addr(alloc);
}

void *mm_realloc(void *ptr, size_t size) {
    
    if (ptr == NULL) {
        // equivalent to malloc
        return mm_malloc(size);

    } else if (size == 0) {
        // equivalent to free
        mm_free(ptr);
        return NULL;

    } else {
        BlockHeader *optr = (BlockHeader *)ptr - 1;
        BlockHeader *nptr;
        BlockHeader *nxtptr;

        void *new_ptr;

        size_t required_size = required_block_size(size);
        size_t nsize;
        size_t copy_size; 

        copy_size = get_size(optr) - 8;

        if(required_size == copy_size){
            return get_payload_addr(optr);
        }

        if(required_size < copy_size){
            if(copy_size - required_size <= 16){
                return get_payload_addr(optr);
            }
            int req = required_size+8;
            set_header(optr, req, 1);
            set_footer(optr, req, 1);
            nptr = get_next(optr);
            int req2 = copy_size - required_size;
            set_header(nptr, req2, 0);
            set_footer(nptr, req2, 0);
            free_list_insert(nptr);
            free_coalesce(nptr);
            return get_payload_addr(optr);
        }
        
        nxtptr = get_next(optr);

        if(nxtptr != NULL && !get_allocated(nxtptr)){
            nsize = get_size(nxtptr);
            if(nsize + copy_size >= required_size){
                free_list_remove(nxtptr);
                if(nsize + copy_size - required_size <= 8){
                    int req = copy_size + 8 + nsize;
                    set_header(optr, req, 1);
                    set_footer(optr, req, 1);
                    return get_payload_addr(optr);
                }
                else{
                    int req = required_size + 8;
                    set_header(optr, req, 1);
                    set_footer(optr, req, 1);
                    nptr = get_next(optr);
                    int req2 = copy_size + nsize - required_size;
                    set_header(nptr, req2, 0);
                    set_footer(nptr, req2, 0);
                    free_list_insert(nptr);
                    free_coalesce(nptr);
                    return get_payload_addr(optr);
                }
            }
        }

        new_ptr = mm_malloc(size);
        if(new_ptr == NULL){
            return NULL;
        }
        memcpy(new_ptr, ptr, copy_size);
        mm_free(ptr);

        // TODO: return pointer to payload of new block
        return new_ptr;
    }
}