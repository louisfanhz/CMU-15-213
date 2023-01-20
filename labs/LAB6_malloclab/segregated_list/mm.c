/*
 * mm.c - implementation using segregated free list. Each block has
 *        minimum size of 4 words(16 bytes) in order to store the
 *        size, alloc, predecessor pointer and successor pointer.
 *        There are mutiple free lists which hold different sizes
 *        of freed blocks. See get_rootno for specification.
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
void *rootlist;

/* helper functions */
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
static void cut_bp(int rootno, void *succp, void *predp);
static void insert_new_root(void *bp);
static int get_rootno(size_t size);

/* checker */
//static void mm_print_freelist(char *func);
//static void mm_print_segregated(char *func);

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
#define CHUNKSIZE (1<<11) /* extend heap by this amount if needed */
#define HDR_SIZE 16
#define NUM_LIST 8

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
#define GET_ROOT(list, n)   ((void *)(*((unsigned int *)(list)+(n))))
#define PUT_ROOT(list, n, bp)   (*((unsigned int *)(list)+(n)) = (unsigned int)(bp))

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
//#define PRINT_FREELIST(func)    mm_print_freelist(func);
#define PRINT_FREELIST(func)

/* The following build uses explicit free list */
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* create and initialize segregated list */
    if ((rootlist = mem_sbrk(NUM_LIST*WSIZE)) == (void *)-1)
        return -1;
    for (int i=0; i < NUM_LIST; i++)
        *((unsigned int *)rootlist+i) = (unsigned int)NULL;
    
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));

    heap_listp += (2*WSIZE);

    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;

    return 0;
}

/* 
 * mm_malloc - allocate 'size' byte of memory on heap, adjusted size
 *             is calculated from size required by user, so to ensure
 *             allocated block has a size larger than minimum isze
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
 *           at the beginning of appropriate free list. The freed block 
 *           must have been allocated by calling mm_malloc
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    void *new_bp;

    /* mark the block as freed */
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));

    /* coalesce neighboring free block(s) */
    new_bp = coalesce(ptr);
    insert_new_root(new_bp);

    PRINT_FREELIST("free");
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
    void *new_bp;

    /* Allocate a multiple of 4 words space to maintain alignment*/
    size = (words % 4) ? (((words+3)/4) * 4 * WSIZE) : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    
    /* Initialize free block header/footer, next/prev and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));           /* New free block header */
    PUT(FTRP(bp), PACK(size, 0));           /* New free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));   /* New epilogue header*/

    /* Coalesce if the previous block was free, and update root */
    new_bp = coalesce(bp);
    insert_new_root(new_bp);

    PRINT_FREELIST("extend_heap");

    return new_bp;
}

/*
 * coalesce - coalesce neighboring free block(s)
 */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    int rootno;

    if (prev_alloc && next_alloc) {
        /* do nothing */
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

        /* if the neighbor is neither first nor last in its free list */
        if (pred_bp != NULL && succ_bp != NULL) {
            PUT_ADR(SUCC(pred_bp), succ_bp);
            PUT_ADR(PRED(succ_bp), pred_bp);
        }
        /* if the neighbor is the first free block in its free list */
        else if (pred_bp == NULL && succ_bp != NULL) {
            rootno = get_rootno(GET_SIZE(HDRP(nxtbp)));
            PUT_ROOT(rootlist, rootno, succ_bp);
            PUT_ADR(PRED(succ_bp), NULL);
        }
        /* if the neighbor is the last in its free list */
        else if (pred_bp != NULL && succ_bp == NULL) {
            PUT_ADR(SUCC(pred_bp), NULL);
        }
        /* if the neighbor is the only block in its free list */
        else {
            rootno = get_rootno(GET_SIZE(HDRP(nxtbp)));
            PUT_ROOT(rootlist, rootno, NULL);
        }
    }
    else if (!prev_alloc && next_alloc) {
        /* get rootno of the previous block before modifying bp */
        rootno = get_rootno(GET_SIZE(HDRP(PREV_BLKP(bp))));
        /* coalesce blocks (get pred and succ using new bp) */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);

        /* get predecessor and successor */
        void *pred_bp = (PRED(bp) == NULL) ? NULL : GET_ADR(PRED(bp));
        void *succ_bp = (SUCC(bp) == NULL) ? NULL : GET_ADR(SUCC(bp));

        /* if the neighbor is neither first nor last in its free list */
        if (pred_bp != NULL && succ_bp != NULL) {
            PUT_ADR(SUCC(pred_bp), succ_bp);
            PUT_ADR(PRED(succ_bp), pred_bp);
        }
        /* if the neighbor is the first free block in the free list */
        else if (pred_bp == NULL && succ_bp != NULL) {
            PUT_ROOT(rootlist, rootno, succ_bp);
            PUT_ADR(PRED(succ_bp), NULL);
        }
        /* if the neighbor is the last in the free list */
        else if (pred_bp != NULL && succ_bp == NULL) {
            PUT_ADR(SUCC(pred_bp), NULL);
        }
        /* if the neighbor is the only block in the free list */
        else {
            PUT_ROOT(rootlist, rootno, NULL);
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
        int rootno1 = get_rootno(GET_SIZE(HDRP(prebp)));
        int rootno2 = get_rootno(GET_SIZE(FTRP(nxtbp)));

        /* coalesce blocks */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
                GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);

        /* block1 is at the first of its free list */
        if (pred_bp1 == NULL && succ_bp1 != NULL) {
            /* if block2 is right after block1 in the same free list */
            if (EQUAL_ADR(pred_bp2, bp) && succ_bp2 != NULL) {
                PUT_ROOT(rootlist, rootno1, succ_bp2);
                PUT_ADR(PRED(succ_bp2), NULL);
            }
            /* if block2 is right afrer block1 and is the last block */
            else if (EQUAL_ADR(pred_bp2, bp) && succ_bp2 == NULL) {
                PUT_ROOT(rootlist, rootno1, NULL);
            }
            /* block2 is in other free list */
            else {
                PUT_ROOT(rootlist, rootno1, succ_bp1);
                PUT_ADR(PRED(succ_bp1), NULL);
                cut_bp(rootno2, succ_bp2, pred_bp2);
            }
        }
        /* block1 is at the middle of its free list */
        else if (pred_bp1 != NULL && succ_bp1 != NULL) {
            /* if block2 is right before block1 in the same free list */
            if (EQUAL_ADR(succ_bp2, bp) && pred_bp2 != NULL) {
                PUT_ADR(SUCC(pred_bp2), succ_bp1);
                PUT_ADR(PRED(succ_bp1), pred_bp2);
            }
            /* if block2 is right after block1 in the same free list */
            else if (EQUAL_ADR(pred_bp2, bp) && succ_bp2 != NULL) {
                PUT_ADR(SUCC(pred_bp1), succ_bp2);
                PUT_ADR(PRED(succ_bp2), pred_bp1);
            }
            /* if block2 is right before block1 and is the first block */
            else if (EQUAL_ADR(succ_bp2, bp) && pred_bp2 == NULL) {
                PUT_ROOT(rootlist, rootno2, succ_bp1);
                PUT_ADR(PRED(succ_bp1), NULL);
            }
            /* if block2 is right after block1 and is the last block */
            else if (EQUAL_ADR(pred_bp2, bp) && succ_bp2 == NULL) {
                PUT_ADR(SUCC(pred_bp1), NULL);
            }
            /* block2 is in other free list */
            else {
                PUT_ADR(SUCC(pred_bp1), succ_bp1);
                PUT_ADR(PRED(succ_bp1), pred_bp1);
                cut_bp(rootno2, succ_bp2, pred_bp2);
            }
        }
        /* block1 is at the end of its free list */
        else if (pred_bp1 != NULL && succ_bp1 == NULL) {
            /* if block2 is right before block1 in the same free list */
            if (EQUAL_ADR(succ_bp2, bp) && pred_bp2 != NULL) {
                PUT_ADR(SUCC(pred_bp2), NULL);
            }
            /* if block2 is right before block1 and is the first block */
            else if (EQUAL_ADR(succ_bp2, bp) && pred_bp2 == NULL) {
                PUT_ROOT(rootlist, rootno2, NULL);
            }
            /* block2 is in other free list */
            else {
                PUT_ADR(SUCC(pred_bp1), NULL);
                cut_bp(rootno2, succ_bp2, pred_bp2);
            }
        }
        /* block1 is the only block in its free list */
        /* block2 is in some other free list */
        else {
            PUT_ROOT(rootlist, rootno1, NULL);
            cut_bp(rootno2, succ_bp2, pred_bp2);
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
    int rootno;

    for (rootno = get_rootno(asize); rootno < NUM_LIST; rootno++) {
        for (bp = GET_ROOT(rootlist, rootno); bp != NULL; bp = GET_ADR(SUCC(bp))) {
            if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
                return bp;
        }
    }
    return NULL;    /* No fit */
}

/*
 * place - allocate space of required size at the designated block
 */
static void place(void *bp, size_t asize)
{
    size_t bsize = GET_SIZE(HDRP(bp));
    int rootno = get_rootno(bsize);
    void *pred_bp = (PRED(bp) == NULL) ? NULL : GET_ADR(PRED(bp));
    void *succ_bp = (SUCC(bp) == NULL) ? NULL : GET_ADR(SUCC(bp));

    if ((bsize-asize) >= (3*DSIZE)) {   /* remainder >= minimum size */
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        /* split and get a new free block */
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(bsize-asize, 0));
        PUT(FTRP(bp), PACK(bsize-asize, 0));

        /* if placed block was neither the first nor the last free block */
        if (pred_bp != NULL && succ_bp != NULL) {
            PUT_ADR(SUCC(pred_bp), succ_bp);
            PUT_ADR(PRED(succ_bp), pred_bp);
        }
        /* if placed block was the first block in the free list */
        else if (pred_bp == NULL && succ_bp != NULL) {
            PUT_ROOT(rootlist, rootno, succ_bp);
            PUT_ADR(PRED(succ_bp), NULL);
        }
        /* if placed block was the last block in the free list */
        else if (pred_bp != NULL && succ_bp == NULL) {
            PUT_ADR(SUCC(pred_bp), NULL);
        }
        /* if placed block was the only free block in the free list */
        else {
            PUT_ROOT(rootlist, rootno, NULL);
        }
        insert_new_root(bp);
    }
    else {
        PUT(HDRP(bp), PACK(bsize, 1));
        PUT(FTRP(bp), PACK(bsize, 1));

        /* if placed block was neither the first nor the last free block */
        if (pred_bp != NULL && succ_bp != NULL) {
            PUT_ADR(SUCC(pred_bp), succ_bp);
            PUT_ADR(PRED(succ_bp), pred_bp);
        }
        /* if placed block was the first block in the free list */
        else if (pred_bp == NULL && succ_bp != NULL) {
            PUT_ADR(PRED(succ_bp), NULL);
            PUT_ROOT(rootlist, rootno, succ_bp);
        }
        /* if placed block was the last block in the free list */
        else if (pred_bp != NULL && succ_bp == NULL) {
            PUT_ADR(SUCC(pred_bp), NULL);
        }
        /* if placed block was the only free block in the free list */
        else {
            PUT_ROOT(rootlist, rootno, NULL);
        }
    }
    PRINT_FREELIST("place");
}

/*
 * cut_bp - cut a freed block out from its free list, updating all
 *          relative fields (predecessors, successors, etc.)
 */
static void cut_bp(int rootno, void *succp, void *predp) {
    /* if block is at the first of its free list */
    if (predp == NULL && succp != NULL) {
        PUT_ROOT(rootlist, rootno, succp);
        PUT_ADR(PRED(succp), NULL);
    }
    /* if block is at the middle of its free list */
    else if (predp != NULL && succp != NULL) {
        PUT_ADR(SUCC(predp), succp);
        PUT_ADR(PRED(succp), predp);
    }
    /* if block is at the end of its free list */
    else if (predp != NULL && succp == NULL) {
        PUT_ADR(SUCC(predp), NULL);
    }
    /* if block is the only block of its free list */
    else {
        PUT_ROOT(rootlist, rootno, NULL);
    }
}

/*
 * insert_new_root - 
 */
static void insert_new_root(void *bp) {
    size_t size = GET_SIZE(HDRP(bp));
    int rootno = get_rootno(size);
    void *root = GET_ROOT(rootlist, rootno);

    /* put new free block at the first of appropriate free list */
    PUT_ADR(SUCC(bp), root);
    PUT_ADR(PRED(bp), NULL);
    if (root != NULL)
        PUT_ADR(PRED((root)), bp);
    PUT_ROOT(rootlist, rootno, bp);

}

/*
 * get_rootno - given the size of new free block, return the appropriate free 
 *              list number of which it should go. This function assumes that
 *              only legal size is set as its argument
 */
static int get_rootno(size_t size) {
    /* words must be even due to alignment requirement */
    size_t words = (size-HDR_SIZE) >> 2;    /* divide by word size (4) */ 
    switch(words) {
        case 2:
            return 0;
        case 4:
            return 1;
        case 6:
        case 8:
            return 2;
        case 10:
        case 12:
        case 14:
        case 16:
            return 3;
    }
    if (words <= 32)
        return 4;
    if (words <= 64)
        return 5;
    if (words <= 128)
        return 6;
    return 7;
}


/********************************************************
 End of helper functions
********************************************************/

/********************************************************
 Heap consistency checker
********************************************************/
/*
static void mm_print_segregated(char *func)
{
    void *bp;
    void *root;

    printf("==========================================\n");
    printf("segregated lists after calling %s:\n", func);
    for (int i=0; i < NUM_LIST; i++) {
        root = GET_ROOT(rootlist, i);
        printf("root = 0x%x", (unsigned int)root);
        for (bp = root; bp != NULL; bp = GET_ADR(SUCC(bp))) {
            printf("{");
            printf("F|");
            printf("%dbyte|", GET_SIZE(HDRP(bp)));
            printf("bp: 0x%x|", (unsigned int)bp);
            printf("succ: 0x%x|", *(unsigned int *)SUCC(bp));
            printf("pred: 0x%x|", *(unsigned int *)PRED(bp));
            printf("F");
            printf("}\n");
        }
        printf("\n");
    }
}
*/

/*
static void mm_print_freelist(char *func)
{
    void *bp;
    void *succ;
    void *pred;

    printf("==========================================\n");
    printf("free list after calling %s:\n", func);

    for (int i=0; i<NUM_LIST; i++) {
        printf("root %d: 0x%x, ", i, (unsigned int)GET_ROOT(rootlist, i));
    }
    printf("\n");
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

    // free list consistency check
    for (int rootno=0; rootno<NUM_LIST; rootno++) {
        bp = GET_ROOT(rootlist, rootno);
        if (bp == NULL)
            continue;
        if (GET_ADR(PRED(bp)) != NULL) {
            printf("0x%x root has a predecessor !!!", (unsigned int)bp);
            exit(-1);
        }
        for (; bp != NULL; bp = succ) {
            succ = GET_ADR(SUCC(bp));
            pred = GET_ADR(PRED(bp));
            if ((pred == NULL && succ != NULL) || (succ == NULL && pred == NULL)) {
                size_t size = GET_SIZE(HDRP(bp));
                int check_rootno = get_rootno(size);
                if (!EQUAL_ADR(bp, GET_ROOT(rootlist, check_rootno))) {
                    printf("0x%x root check failed: rootno: %d, but: %d\n", 
                            (unsigned int)bp, rootno, check_rootno);
                    exit(-1);
                }
            }
            else if (succ == NULL && pred != NULL) {
                if (!EQUAL_ADR(GET_ADR(SUCC(pred)), bp)) {
                    printf("0x%x last block check failed", (unsigned int)bp);
                    exit(-1);
                }
            }
            else {
                if (!EQUAL_ADR(GET_ADR(SUCC(pred)), bp)) {
                    printf("0x%x consistency check failed", (unsigned int)bp);
                    exit(-1);
                }
                if (!EQUAL_ADR(GET_ADR(PRED(succ)), bp)) {
                    printf("0x%x consistency check failed", (unsigned int)bp);
                    exit(-1);
                }
            }
        }
    }
}
*/

