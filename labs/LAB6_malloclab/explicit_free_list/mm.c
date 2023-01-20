/*
 * mm.c - implementation using explicit free list. Each block has
 *        minimum size of 4 words(16 bytes) in order to store the
 *        size, alloc, predecessor pointer and successor pointer.
 *        The start of the free list is pointed by root, and each
 *        search of free block has linear time to the total number
 *        free blocks.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/* Global variables */
void *heap_listp;
void *root;

/* helper functions */
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);

/* checker */
void mm_print_heap(char *func);
void mm_print_freelist(char *func);

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "pddbb",
    /* First member's full name */
    "Haozhi Fan",
    /* First member's email address */
    "louisfanhz@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* define constants */
#define WSIZE 4     /* size of a single word */
#define DSIZE 8     /* size of a double word */
#define ALIGNMENT 8 /* alignment requirement */
#define CHUNKSIZE (1<<12) /* extend heap by this amount if needed */

#define MAX(a, b) ((a) > (b) ? (a) : (b))

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)              (*(unsigned int *)(p))
#define PUT(p, val)         (*(unsigned int *)(p) = (val))
#define GET_ADR(p)          ((void *)(*(unsigned int *)(p)))
#define PUT_ADR(p, val)     (*(unsigned int *)(p) = (unsigned int)(val))
#define EQUAL_ADR(a, b)     ((unsigned int)(a) == (unsigned int)(b))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~0x7)
#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer*/
#define HDRP(bp)    ((char *)(bp) - WSIZE)
#define FTRP(bp)    ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given free block ptr bp, compute address of its successor and predcessor */
#define SUCC(bp)    ((char *)(bp))
#define PRED(bp)    ((char *)(bp) + WSIZE)

/* Given block ptr bp, compute address of next and previous blocks*/
#define NEXT_BLKP(bp)   ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)   ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Checker macro */
//#define PRINT_HEAP(func)    mm_print_freelist(func);
#define PRINT_HEAP(func)

/* The following build uses explicit free list */
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));

    heap_listp += (2*WSIZE);
    root = NULL;    /* initializes root as NULL */

    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;

    return 0;
}

/* 
 * mm_malloc - 
 *            
 */
void *mm_malloc(size_t size)
{
    size_t asize;       /* adjusted block size */
    size_t extendsize;  /* amount to extend heap if no fit */
    char *bp;

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* adjust block size to include overhead and alignment */
    if (size <= DSIZE)
        asize = 3*DSIZE;
    else
        asize = DSIZE * ((size+ (2*DSIZE) + (DSIZE-1)) / DSIZE);

    /* search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* no fit found. request more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

/*
 * mm_free - frees a block and uses boundary-tag coalescing to merge
 *           it with any adjacent free blocks. Insert the freed block
 *           at the beginning of the free list. The freed block must 
 *           have been allocated by calling mm_malloc
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));

    /* mark the block as freed */
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));

    /* coalesce neighboring free block(s) */
    root = coalesce(ptr);
    if (GET_ADR(SUCC(root)) != NULL)
        PUT_ADR(PRED(GET_ADR(SUCC(root))), root);

    PRINT_HEAP("mm_free");
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copysize;

    if (ptr == NULL) {
        return mm_malloc(size);
    }
    else if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    copysize = (GET_SIZE(HDRP(oldptr)) <= size) ? 
                GET_SIZE(HDRP(oldptr)) : size;
    newptr = mm_malloc(size);
    memcpy(newptr, oldptr, copysize);
    mm_free(oldptr);
    return newptr;
}

/********************************************************
 Helper functions
********************************************************/
/*
 * extend_heap - extends the heap with a new free block
 */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate a multiple of 4 words space to maintain alignment*/
    size = (words % 4) ? (((words+3)/4) * 4 * WSIZE) : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    
    /* Initialize free block header/footer, next/prev and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));           /* New free block header */
    PUT(FTRP(bp), PACK(size, 0));           /* New free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));   /* New epilogue header*/

    /* Coalesce if the previous block was free, and update root */
    root = coalesce(bp);

    /* update old root prev if new block is not the first block */
    if (GET_ADR(SUCC(root)) != NULL)                  
        PUT_ADR(PRED(GET_ADR(SUCC(root))), root);

    PRINT_HEAP("extend_heap");

    return root;
}

/*
 * coalesce - coalesce neighboring free block(s)
 */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {
        PUT_ADR(SUCC(bp), root);
        PUT_ADR(PRED(bp), NULL);
    }
    else if (prev_alloc && !next_alloc) {
        /* get predecessor and successor */
        void *nxtbp = NEXT_BLKP(bp);
        void *pred_bp = (PRED(nxtbp) == NULL) ? NULL : GET_ADR(PRED(nxtbp));
        void *succ_bp = (SUCC(nxtbp) == NULL) ? NULL : GET_ADR(SUCC(nxtbp));

        /* coalesce blocks */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));

        /* if the neighbor is neither first nor last in the free list */
        if (pred_bp != NULL && succ_bp != NULL) {
            PUT_ADR(SUCC(pred_bp), succ_bp);
            PUT_ADR(PRED(succ_bp), pred_bp);
            PUT_ADR(SUCC(bp), root);
            PUT_ADR(PRED(bp), NULL);
        }
        /* if the neighbor is the first free block in the free list */
        else if (pred_bp == NULL && succ_bp != NULL) {
            PUT_ADR(SUCC(bp), succ_bp);
            PUT_ADR(PRED(bp), NULL);
            PUT_ADR(PRED(succ_bp), bp);
        }
        /* if the neighbor is the last in the free list */
        else if (pred_bp != NULL && succ_bp == NULL) {
            PUT_ADR(SUCC(pred_bp), NULL);
            PUT_ADR(SUCC(bp), root);
            PUT_ADR(PRED(bp), NULL);
        }
        /* if the neighbor is the only block in the free list */
        else {
            PUT_ADR(SUCC(bp), NULL);
            PUT_ADR(PRED(bp), NULL);
        }
    }
    else if (!prev_alloc && next_alloc) {
        /* coalesce blocks (get pred and succ using new bp) */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);

        /* get predecessor and successor */
        void *pred_bp = (PRED(bp) == NULL) ? NULL : GET_ADR(PRED(bp));
        void *succ_bp = (SUCC(bp) == NULL) ? NULL : GET_ADR(SUCC(bp));

        /* if the neighbor is neither first nor last in the free list */
        if (pred_bp != NULL && succ_bp != NULL) {
            PUT_ADR(SUCC(pred_bp), succ_bp);
            PUT_ADR(PRED(succ_bp), pred_bp);
            PUT_ADR(SUCC(bp), root);
            PUT_ADR(PRED(bp), NULL);
        }
        /* if the neighbor is the first free block in the free list */
        else if (pred_bp == NULL && succ_bp != NULL) {
            /*do nothing*/
        }
        /* if the neighbor is the last in the free list */
        else if (pred_bp != NULL && succ_bp == NULL) {
            PUT_ADR(SUCC(pred_bp), NULL);
            PUT_ADR(SUCC(bp), root);
            PUT_ADR(PRED(bp), NULL);
        }
        /* if the neighbor is the only block in the free list */
        else {
            /* do nothing */
        }
    }
    else {  /* coalescing block which has two free blocks neighbor*/
        /* get predecessors and successers */
        void *prebp = PREV_BLKP(bp);
        void *nxtbp = NEXT_BLKP(bp);
        /* FIXME this conditionals are unnecessary, if im correct */
        void *pred_bp1 = (PRED(prebp) == NULL) ? NULL : GET_ADR(PRED(prebp));
        void *succ_bp1 = (SUCC(prebp) == NULL) ? NULL : GET_ADR(SUCC(prebp));
        void *pred_bp2 = (PRED(nxtbp) == NULL) ? NULL : GET_ADR(PRED(nxtbp));
        void *succ_bp2 = (SUCC(nxtbp) == NULL) ? NULL : GET_ADR(SUCC(nxtbp));

        /* coalesce blocks */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
                GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);

        /* if block1 is first, block2 is last in free list */
        /* or if block1 is last, block2 is first in free list */
        if ((pred_bp1 == NULL && succ_bp1 != NULL
         && pred_bp2 != NULL && succ_bp2 == NULL)
         || (pred_bp1 != NULL && succ_bp1 == NULL
         &&  pred_bp2 == NULL && succ_bp2 != NULL)) {
            PUT_ADR(SUCC(bp), NULL);
            PUT_ADR(PRED(bp), NULL);
        }
        /* if block1 is first, block2 is in the middle */
        else if (pred_bp1 == NULL && succ_bp1 != NULL
              && pred_bp2 != NULL && succ_bp2 != NULL) {
            /* if block1 is first, block2 is the second in the free list */
            if (EQUAL_ADR(bp, pred_bp2)) {
                PUT_ADR(SUCC(bp), succ_bp2);
                PUT_ADR(PRED(bp), NULL);
                PUT_ADR(PRED(succ_bp2), bp);
            }
            else {
                PUT_ADR(SUCC(pred_bp2), succ_bp2);
                PUT_ADR(PRED(succ_bp2), pred_bp2);
            }
        }
        /* if block1 is last, block2 is in the middle */
        else if (pred_bp1 != NULL && succ_bp1 == NULL
              && pred_bp2 != NULL && succ_bp2 != NULL) {
            /* if block 1 is last, block 2 is the second to the last */
            if (EQUAL_ADR(bp, succ_bp2)) {
                PUT_ADR(SUCC(bp), root);
                PUT_ADR(PRED(bp), NULL);
                PUT_ADR(SUCC(pred_bp2), NULL);
            }
            else {
                PUT_ADR(SUCC(bp), root);
                PUT_ADR(PRED(bp), NULL);
                PUT_ADR(SUCC(pred_bp1), NULL);
                PUT_ADR(SUCC(pred_bp2), succ_bp2);
                PUT_ADR(PRED(succ_bp2), pred_bp2);
            }
        }
        /* if block2 is first, block1 is in the middle */
        else if (pred_bp2 == NULL && succ_bp2 != NULL
              && pred_bp1 != NULL && succ_bp1 != NULL) {
            /* if block2 is first, block1 is second in the free list */
            if (EQUAL_ADR(succ_bp2, bp)) {
                PUT_ADR(SUCC(bp), succ_bp1);
                PUT_ADR(PRED(bp), NULL);
                PUT_ADR(PRED(succ_bp1), bp);
            }
            else {
                PUT_ADR(SUCC(bp), succ_bp2);
                PUT_ADR(PRED(bp), NULL);
                /* FIXME this line can be deleted? */
                PUT_ADR(PRED(succ_bp2), bp);
                PUT_ADR(SUCC(pred_bp1), succ_bp1);
                PUT_ADR(PRED(succ_bp1), pred_bp1);
            }
        }
        /* if block2 is last, block1 is in the middle */
        else if (pred_bp2 != NULL && succ_bp2 == NULL
              && pred_bp1 != NULL && succ_bp1 != NULL) {
            /* if block2 is last, block1 is second to the last */
            if (EQUAL_ADR(pred_bp2, bp)) {
                PUT_ADR(SUCC(bp), root);
                PUT_ADR(PRED(bp), NULL);
                PUT_ADR(SUCC(pred_bp1), NULL);
            }
            else {
                PUT_ADR(SUCC(bp), root);
                PUT_ADR(PRED(bp), NULL);
                PUT_ADR(SUCC(pred_bp2), NULL);
                PUT_ADR(SUCC(pred_bp1), succ_bp1);
                PUT_ADR(PRED(succ_bp1), pred_bp1);
            }
        }
        /* if both block1 and block2 are in the middle */
        else if (pred_bp1 != NULL && succ_bp1 != NULL
              && pred_bp2 != NULL && succ_bp2 != NULL) {
            /* if block1 is the predecessor of block2 */
            if (EQUAL_ADR(bp, pred_bp2)) {
                PUT_ADR(SUCC(pred_bp1), succ_bp2);
                PUT_ADR(PRED(succ_bp2), pred_bp1);
                PUT_ADR(SUCC(bp), root);
                PUT_ADR(PRED(bp), NULL);
            }
            /* if block1 is the successor of block 2 */
            else if (EQUAL_ADR(bp, succ_bp2)) {
                PUT_ADR(SUCC(pred_bp2), succ_bp1);
                PUT_ADR(PRED(succ_bp1), pred_bp2);
                PUT_ADR(SUCC(bp), root);
                PUT_ADR(PRED(bp), NULL);
            }
            /* if block1 and block2 are far away from each other */
            else {
                PUT_ADR(SUCC(bp), root);
                PUT_ADR(PRED(bp), NULL);
                PUT_ADR(SUCC(pred_bp1), succ_bp1);
                PUT_ADR(PRED(succ_bp1), pred_bp1);
                PUT_ADR(SUCC(pred_bp2), succ_bp2);
                PUT_ADR(PRED(succ_bp2), pred_bp2);
            }
        }
    }

    return bp;
}

/*
 * find_fit - traverse the free list to find block large enough to hold
 *            both overhead and payload, satisfying alignment requirement
 */
static void *find_fit(size_t asize)
{
    void *bp;

    for (bp = root; bp != NULL; bp = GET_ADR(SUCC(bp))) {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
            return bp;
    }
    return NULL;    /* No fit */
}

/*
 * place - allocate space of required size at the designated block
 */
static void place(void *bp, size_t asize)
{
    size_t bsize = GET_SIZE(HDRP(bp));
    void *pred_bp = (PRED(bp) == NULL) ? NULL : GET_ADR(PRED(bp));
    void *succ_bp = (SUCC(bp) == NULL) ? NULL : GET_ADR(SUCC(bp));

    if ((bsize-asize) >= (3*DSIZE)) {   /* remainder >= minimum size */
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        /* split and get a new free block */
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(bsize-asize, 0));
        PUT(FTRP(bp), PACK(bsize-asize, 0));

        /* if placed block is neither the first nor the last free block */
        if (pred_bp != NULL && succ_bp != NULL) {
            PUT_ADR(SUCC(pred_bp), succ_bp);
            PUT_ADR(PRED(succ_bp), pred_bp);
            PUT_ADR(PRED(root), bp);
            PUT_ADR(SUCC(bp), root);
            PUT_ADR(PRED(bp), NULL);
            root = bp;
        }
        /* if placed block was the first block in the free list */
        else if (pred_bp == NULL && succ_bp != NULL) {
            PUT_ADR(PRED(succ_bp), bp);
            PUT_ADR(SUCC(bp), succ_bp);
            PUT_ADR(PRED(bp), NULL);
            root = bp;
        }
        /* if placed block was the last block in the free list */
        else if (pred_bp != NULL && succ_bp == NULL) {
            PUT_ADR(SUCC(pred_bp), NULL);
            PUT_ADR(PRED(bp), NULL);
            PUT_ADR(SUCC(bp), root);
            PUT_ADR(PRED(root), bp);
            root = bp;
        }
        /* if placed block was the only free block in the free list */
        else {
            PUT_ADR(SUCC(bp), NULL);
            PUT_ADR(PRED(bp), NULL);
            root = bp;
        }
    }
    else {
        PUT(HDRP(bp), PACK(bsize, 1));
        PUT(FTRP(bp), PACK(bsize, 1));

        /* if placed block is neither the first nor the last free block */
        if (pred_bp != NULL && succ_bp != NULL) {
            PUT_ADR(SUCC(pred_bp), succ_bp);
            PUT_ADR(PRED(succ_bp), pred_bp);
        }
        /* if placed block was the first block in the free list */
        else if (pred_bp == NULL && succ_bp != NULL) {
            PUT_ADR(PRED(succ_bp), NULL);
            root = succ_bp;
        }
        /* if placed block was the last block in the free list */
        else if (pred_bp != NULL && succ_bp == NULL) {
            PUT_ADR(SUCC(pred_bp), NULL);
        }
        /* if placed block was the only free block in the free list */
        else {
            root = NULL;
        }
    }

    PRINT_HEAP("place");
}

/********************************************************
 End of helper functions
********************************************************/

/********************************************************
 Heap consistency checker
********************************************************/
/*
 *  mm_print_heap - print the entire heap to standard out, including allocated
 *                  blocks and free blocks
 */
void mm_print_heap(char *func)
{
    void *bp;

    printf("==========================================\n");
    printf("heap after calling %s:\n", func);

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        printf("{");
        if (GET_ALLOC(HDRP(bp)))
            printf("A|");
        else
            printf("F|");
        printf("%dbyte|", GET_SIZE(HDRP(bp)));
        printf("bp: 0x%x|", (unsigned int)bp);
        printf("succ: 0x%x|", *(unsigned int *)SUCC(bp));
        printf("pred: 0x%x|", *(unsigned int *)PRED(bp));
        if (GET_ALLOC(FTRP(bp)))
            printf("A");
        else
            printf("F");
        printf("}\n");
    }
}

void mm_print_freelist(char *func) 
{
    void *bp;

    printf("==========================================\n");
    printf("free list after calling %s:\n", func);
    printf("root = 0x%x\n", (unsigned int)root);

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (GET_ALLOC(HDRP(bp)))
            continue;
        printf("{");
        printf("F|");
        printf("%dbyte|", GET_SIZE(HDRP(bp)));
        printf("bp: 0x%x|", (unsigned int)bp);
        printf("succ: 0x%x|", *(unsigned int *)SUCC(bp));
        printf("pred: 0x%x|", *(unsigned int *)PRED(bp));
        printf("F");
        printf("}\n");
    }
}






