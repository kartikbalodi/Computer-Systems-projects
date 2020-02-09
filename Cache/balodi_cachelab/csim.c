#include <getopt.h>  // getopt, optarg
#include <stdlib.h>  // exit, atoi, malloc, free
#include <stdio.h>   // printf, fprintf, stderr, fopen, fclose, FILE
#include <limits.h>  // ULONG_MAX
#include <string.h>  // strcmp, strerror
#include <errno.h>   // errno

/* fast base-2 integer logarithm */
#define INT_LOG2(x) (31 - __builtin_clz(x))
#define NOT_POWER2(x) (__builtin_clz(x) + __builtin_ctz(x) != 31)

/* tag_bits = ADDRESS_LENGTH - set_bits - block_bits */
#define ADDRESS_LENGTH 64

/**
 * Print program usage (no need to modify).
 */
static void print_usage() {
    printf("Usage: csim [-hv] -K <num> -S <num> -B <num> -t <file>\n");
    printf("Options:\n");
    printf("  -h           Print this help message.\n");
    printf("  -v           Optional verbose flag.\n");
    printf("  -S <num>     Number of sets.           (must be > 0)\n");
    printf("  -K <num>     Number of lines per set.  (must be > 0)\n");
    printf("  -B <num>     Number of bytes per line. (must be > 0)\n");
    printf("  -p <policy>  Eviction policy. (one of 'FIFO', 'LRU')\n");
    printf("  -t <file>    Trace file.\n\n");
    printf("Examples:\n");
    printf("  $ ./csim    -S 16  -K 1 -B 16 -p LRU -t traces/yi.trace\n");
    printf("  $ ./csim -v -S 256 -K 2 -B 16 -p LRU -t traces/yi.trace\n");
    exit(0);
}

/* Parameters set by command-line args (no need to modify) */
int verbose = 0;   // print trace if 1
int S = 0;         // number of sets
int K = 0;         // lines per set
int B = 0;         // bytes per line
int blockOffsetBit = 0;    // log2(B)
int setIndexBit = 0;    // log2(S)

typedef enum { FIFO = 1, LRU = 2 } Policy;
Policy policy;     // 0 (undefined) by default

FILE *trace_fp = NULL;

/**
 * Parse input arguments and set verbose, S, K, B, policy, trace_fp.
 *
 * TODO: Finish implementation
 */
static void parse_arguments(int argc, char **argv) {
    char c;
    while ((c = getopt(argc, argv, "S:K:B:p:t:vh")) != -1) {
        switch(c) {
            case 'S':
                S = atoi(optarg);
                if (NOT_POWER2(S)) {
                    fprintf(stderr, "ERROR: S must be a power of 2\n");
                    exit(1);
                }
                break;
            case 'K':
                // TODO
                K = atoi(optarg);
                break;
            case 'B':
                // TODO
                B = atoi(optarg);
                break;
            case 'p':
                if (!strcmp(optarg, "FIFO")) {
                    policy = FIFO;
                }
                // TODO: parse LRU, exit with error for unknown policy
                else if (!strcmp(optarg, "LRU")){
                    policy = LRU;
                }
                else{
                    fprintf(stderr, "ERROR: Unknown policy\n");
                    exit(1);
                }
                break;
            case 't':
                // TODO: open file trace_fp for reading
                trace_fp = fopen(optarg, "r");
                if (!trace_fp) {
                    fprintf(stderr, "ERROR: %s: %s\n", optarg, strerror(errno));
                    exit(1);
                }
                break;
            case 'v':
                // TODO
                verbose = 1;
                break;
            case 'h':
                // TODO
                print_usage();
                exit(0);
            default:
                print_usage();
                exit(1);
        }
    }

    /* Make sure that all required command line args were specified and valid */
    if (S <= 0 || K <= 0 || B <= 0 || policy == 0) {
        printf("ERROR: Negative or missing command line arguments\n");
        print_usage();
        fclose(trace_fp);
        exit(1);
    }

    /* Other setup if needed */
    blockOffsetBit = INT_LOG2(B);
    setIndexBit = INT_LOG2(S);
}

/**
 * Cache data structures
 * TODO: Define your own!
 */

typedef struct {
    int valid;
    unsigned long tag;
    int usedCountLRU;
    int orderCountFIFO;
} setLine;

// cacheSet is a pointer to an array of size K (lines per set) containing the line structs
typedef struct {
    setLine *lines;
} cacheSet;

// cache consists of a pointer to an array of size S (no. of sets)
typedef struct {
    cacheSet *sets;
} myCache;

//global declaration for the structs so we can void access the cache across argument calls
myCache cache;	
cacheSet set;
setLine line;

/**
 * Allocate cache data structures.
 *
 * This function dynamically allocates (with malloc) data structures for each of
 * the `S` sets and `K` lines per set.
 *
 * TODO: Implement
 */
static void allocate_cache() {
    //dynamically allocating sets within cache
    cache.sets = (cacheSet *) malloc(sizeof(cacheSet) * S);
    //dynamically allocating lines withing sets
    for (int i=0; i<S; i++) 
    {
        cache.sets[i].lines =  (setLine *) malloc(sizeof(setLine) * K);
        //loop to construct the lines
        int j = 0;
        for (j=0; j<K; j++) 
        {
            cache.sets[i].lines[j].valid = 0;
            cache.sets[i].lines[j].tag = 0;
            cache.sets[i].lines[j].usedCountLRU = 0;
            cache.sets[i].lines[j].orderCountFIFO = 0;
        }
    }
}

/**
 * Deallocate cache data structures.
 *
 * This function deallocates (with free) the cache data structures of each
 * set and line.
 *
 * TODO: Implement
 */
static void free_cache() {
    for (int i=0; i<S; i++) {
        free(cache.sets[i].lines);
    }
    free(cache.sets);
}

/* Counters used to record cache statistics */
int miss_count     = 0;
int hit_count      = 0;
int eviction_count = 0;

/* Helper function used in access_data
*/

//method to check if set is full and if not, check next open line index
int findLineIndex(cacheSet checkset){
    for(int i=0; i<K; i++){
        if(checkset.lines[i].valid == 0){
            return i;
        }
    }
    return K; //returns K if set is full
}

//check if line and tag matches (i.e. a hit)
int checkHit(setLine line, unsigned long tag){
    if(line.valid){
        if(line.tag == tag){
            return 1;
        }
    }
    return 0;
}

//find the line index to evict LRU
int findEvictLRU(cacheSet checkset){
    int min = checkset.lines[0].usedCountLRU;
    int index = 0;
    for(int i=0; i<K; i++){
        if(min>checkset.lines[i].usedCountLRU){
            index = i;
            min = checkset.lines[i].usedCountLRU;
        }
    }
    return index;
}

//return the max usedCountLRU value in a set
int retMaxUsedCountLRU(cacheSet checkset){
    int max = checkset.lines[0].usedCountLRU;
    for(int i=0; i<K; i++){
        if(checkset.lines[i].usedCountLRU>max){
            max = checkset.lines[i].usedCountLRU;
        }
    }
    return max;
}

//find the line index to evict FIFO
int findEvictFIFO(cacheSet checkset){
    int max = checkset.lines[0].orderCountFIFO;
    int index = 0;
    for(int i=0; i<K; i++){
        if(checkset.lines[i].orderCountFIFO>max){
            index = i;
            max = checkset.lines[i].orderCountFIFO;
        }
    }
    return index;
}

/**
 * Simulate a memory access.
 *
 * If the line is already in the cache, increase `hit_count`; otherwise,
 * increase `miss_count`; increase `eviction_count` if another line must be
 * evicted. This function also updates the metadata used to implement eviction
 * policies (LRU, FIFO).
 *
 * TODO: Implement
 */
static void access_data(unsigned long addr) {
    //compute the tag size (64 bit system) and the tag itself
    int tagSize = 64-(blockOffsetBit + setIndexBit);
    unsigned long tag = addr >> (blockOffsetBit + setIndexBit);
    //compute set index
    unsigned long temp = addr << (tagSize);
    unsigned long setIndex = temp >> (tagSize + blockOffsetBit);

    if(policy==1){ //FIFO
        int hit = 0;
        for(int i=0; i<K; i++){
            if(checkHit(cache.sets[setIndex].lines[i], tag) == 1){
                //update hit and hit_count for hit
                hit = 1;
                hit_count += 1;
            }
        }
        if(hit == 0 && findLineIndex(cache.sets[setIndex]) == K){ // check if set is full
            //update miss_count, evict_count, change tag and orderCountFIFO
            miss_count += 1;
            eviction_count += 1;
            int evictIndex = findEvictFIFO(cache.sets[setIndex]);
            cache.sets[setIndex].lines[evictIndex].tag = tag;
            cache.sets[setIndex].lines[evictIndex].orderCountFIFO = 0;
            for(int j=0; j<K; j++){
                if(cache.sets[setIndex].lines[j].valid){
                    cache.sets[setIndex].lines[j].orderCountFIFO += 1;
                }
            }
        }
        else if(hit == 0 && findLineIndex(cache.sets[setIndex]) < K){ // check if set is not full
            //update miss_count, change tag, valid bit and increment orderCountFIFO
            miss_count += 1;
            int lineIndex = findLineIndex(cache.sets[setIndex]);
            cache.sets[setIndex].lines[lineIndex].tag = tag;
            cache.sets[setIndex].lines[lineIndex].valid = 1;
            for(int j=0; j<K; j++){
                if(cache.sets[setIndex].lines[j].valid){
                    cache.sets[setIndex].lines[j].orderCountFIFO += 1;
                }
            }
        }
    }
    else if(policy==2){ //LRU
        int hit = 0;
        //check for hit first
        for(int i=0; i<K; i++){
            if(checkHit(cache.sets[setIndex].lines[i], tag) == 1){
                //update hit, hit_count and usedCountLRU for hit
                hit = 1;
                hit_count += 1;
                cache.sets[setIndex].lines[i].usedCountLRU = retMaxUsedCountLRU(cache.sets[setIndex])+1;
            }
        }
        if (hit == 0 && findLineIndex(cache.sets[setIndex]) == K){ // check if set is full
            //update miss_count, evict_count, change tag and usedCountLRU
            miss_count += 1;
            eviction_count += 1;
            int evictIndex = findEvictLRU(cache.sets[setIndex]);
            cache.sets[setIndex].lines[evictIndex].tag = tag;
            cache.sets[setIndex].lines[evictIndex].usedCountLRU = retMaxUsedCountLRU(cache.sets[setIndex])+1;
        }
        else if (hit == 0 && findLineIndex(cache.sets[setIndex]) < K){ // check if set is not full
            //update miss_count, change tag, valid bit and usedCountLRU
            miss_count += 1;
            int lineIndex = findLineIndex(cache.sets[setIndex]);
            cache.sets[setIndex].lines[lineIndex].tag = tag;
            cache.sets[setIndex].lines[lineIndex].valid = 1;
            cache.sets[setIndex].lines[lineIndex].usedCountLRU = retMaxUsedCountLRU(cache.sets[setIndex])+1;
        }
    }
}

/**
 * Replay the input trace.
 *
 * This function:
 * - reads lines (e.g., using fgets) from the file handle `trace_fp` (a global variable)
 * - skips lines not starting with ` S`, ` L` or ` M`
 * - parses the memory address (unsigned long, in hex) and len (unsigned int, in decimal)
 *   from each input line
 * - calls `access_data(address)` for each access to a cache line
 *
 * TODO: Implement
 */
static void replay_trace() {

    char command;
    unsigned long address;
    int size;

    while(fscanf(trace_fp, " %c %lx,%d", &command, &address, &size) == 3){
        switch(command){
            //skip I, only parse S L or M
            case 'I':
                break;
            case 'L':
                access_data(address);
                for(int i=1; i<size; i++){
                    if((address+i)%B == 0){
                        access_data(address+i);
                    }
                }
                break;
            case 'S':
                access_data(address);
                for(int i=1; i<size; i++){
                    if((address+i)%B == 0){
                        access_data(address+i);
                    }
                }
                break;
            case 'M':
                access_data(address);
                access_data(address);
                for(int i=1; i<size; i++){
                    if((address+i)%B == 0){
                        access_data(address+i);
                        access_data(address+i);
                    }
                }
                break;
            default:
                break;
        }
    }

}

/**
 * Print cache statistics (DO NOT MODIFY).
 */
static void print_summary(int hits, int misses, int evictions) {
    printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
}

int main(int argc, char **argv) {
    parse_arguments(argc, argv);  // set global variables used by simulation
    allocate_cache();             // allocate data structures of cache
    replay_trace();               // simulate the trace and update counts
    free_cache();                 // deallocate data structures of cache
    fclose(trace_fp);             // close trace file
    print_summary(hit_count, miss_count, eviction_count);  // print counts
    return 0;
}
