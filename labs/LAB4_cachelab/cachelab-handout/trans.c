/*****************************************
 Haozhi Fan
 2021/03/15
 Cache Lab Part B
*****************************************/

/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"
#include <stdlib.h>

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, i1, j1;
    int HALFBLK = 4;
    int BLK = 8;
    int PADD = 13;
    int temp[N+PADD][M+PADD]; 

    if (N == 32) {
        for (i = 0; i < N; i+=BLK) {
            for (j = 0; j < M; j+=BLK) {
                // store the diagonal blocks in a buffer, buffer
                // has larger size so data is cached in different sets
                if (i == j) {
                    for (i1 = i; i1 < i+BLK; i1++) {
                        for (j1 = j; j1 < j+BLK; j1++) {
                            temp[PADD+i1][PADD+j1] = A[i1][j1];
                        }
                    }  
                    for (i1 = i; i1 < i+BLK; i1++) {
                        for (j1 = j; j1 < j+BLK; j1++) {
                            B[j1][i1] = temp[PADD+i1][PADD+j1];
                        }
                    } 
                    continue;
                }
                for (i1 = i; i1 < i+BLK; i1++) {
                    for (j1 = j; j1 < j+BLK; j1++) {
                            B[j1][i1] = A[i1][j1];
                    }
                }
            }
        }
    }
    else if (N == 64) {
        for (i = 0; i < N; i+=BLK) {
            for (j = 0; j < M; j+=BLK) {
                int mid = i+HALFBLK;
                // store 1/4 (upper left) of a block of A in temp
                for (i1 = i; i1 < mid; i1++)
                    for (j1 = j; j1 < j+HALFBLK; j1++)
                        temp[PADD+i1][PADD+j1] = A[i1][j1];
                // transpose the upper right 4*4 elements of A and store in B
                for (i1 = i; i1 < mid; i1++)
                    for (j1 = j+HALFBLK; j1 < j+BLK; j1++)
                        B[j1][i1] = A[i1][j1];
                // transpose the lower right 4*4 elements of A and store in B
                for (i1 = mid; i1 < i+BLK; i1++)
                    for (j1 = j+HALFBLK; j1 < j+BLK; j1++)
                        B[j1][i1] = A[i1][j1];
                // transpose the lower left 4*4 elements of A and store in B
                for (i1 = mid; i1 < i+BLK; i1++)
                    for (j1 = j; j1 < j+HALFBLK; j1++)
                        B[j1][i1] = A[i1][j1];
                // transpose and move the elements from temp to B
                for (i1 = i; i1 < mid; i1++)
                    for (j1 = j; j1 < j+HALFBLK; j1++)
                        B[j1][i1] = temp[PADD+i1][PADD+j1];
            }
        }
    }
    else {
        for (i = 0; i < N; i+=16)
            for (j = 0; j < M; j+= 16)
                for (i1 = i; i1 < i+16 && i1 < N; i1++)
                    for (j1 = j; j1 < j+16 && j1 < M; j1++)
                        B[j1][i1] = A[i1][j1];
    }

}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/* 
 * use blocking: block size B*B = 8ints*8ints, so the blocks divide the
 * 32*32 matrix exactly
 */ 
char blocking_8by8_desc[] = "use 8*8 blocking";
void blocking_8by8(int M, int N, int A[N][M], int B[M][N])
{

    int i, j, i1, j1;
    int BLK = 8;
    int PADD = 13;
    int temp[N+PADD][M+PADD];
    /*
        temp is stored on stack, is the data on stack stored in L1 cache?
        I don't know, but for the sake of this lab, it dosen't seem to
        create conflict misses when loading/writting A/B.
    */

    for (i = 0; i < N; i+=BLK) {
        for (j = 0; j < M; j+=BLK) {
            // store the diagonal blocks in a buffer, buffer
            // has larger size so data is cached in different sets
            if (i == j) {
                for (i1 = i; i1 < i+BLK; i1++) {
                    for (j1 = j; j1 < j+BLK; j1++) {
                        temp[PADD+i1][PADD+j1] = A[i1][j1];
                    }
                }  
                for (i1 = i; i1 < i+BLK; i1++) {
                    for (j1 = j; j1 < j+BLK; j1++) {
                        B[j1][i1] = temp[PADD+i1][PADD+j1];
                    }
                } 
                continue;
            }
            for (i1 = i; i1 < i+BLK; i1++) {
                for (j1 = j; j1 < j+BLK; j1++) {
                        B[j1][i1] = A[i1][j1];
                }
            }
        }
    }
}

/* 
 * use blocking similar to 32*32 case, but with some transformation about the blocks
 */ 
char blocking_8by8_64_desc[] = "use 8*8 blocking";
void blocking_8by8_64(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, i1, j1;
    int HALFBLK = 4;
    int BLK = 8;
    int PADD = 13;
    int temp[N+PADD][M+PADD];      // decalre buffer matrix, check the size
                                   // probably a problem FIXME
    for (i = 0; i < N; i+=BLK) {
        for (j = 0; j < M; j+=BLK) {
            int mid = i+HALFBLK;
            // store 1/4 (upper left) of a block of A in temp
            for (i1 = i; i1 < mid; i1++)
                for (j1 = j; j1 < j+HALFBLK; j1++)
                    temp[PADD+i1][PADD+j1] = A[i1][j1];
            // transpose the upper right 4*4 elements of A and store in B
            for (i1 = i; i1 < mid; i1++)
                for (j1 = j+HALFBLK; j1 < j+BLK; j1++)
                    B[j1][i1] = A[i1][j1];
            // transpose the lower right 4*4 elements of A and store in B
            for (i1 = mid; i1 < i+BLK; i1++)
                for (j1 = j+HALFBLK; j1 < j+BLK; j1++)
                    B[j1][i1] = A[i1][j1];
            // transpose the lower left 4*4 elements of A and store in B
            for (i1 = mid; i1 < i+BLK; i1++)
                for (j1 = j; j1 < j+HALFBLK; j1++)
                    B[j1][i1] = A[i1][j1];
            // transpose and move the elements from temp to B
            for (i1 = i; i1 < mid; i1++)
                for (j1 = j; j1 < j+HALFBLK; j1++)
                    B[j1][i1] = temp[PADD+i1][PADD+j1];
        }
    }

}


/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    // registerTransFunction(trans, trans_desc); 
}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

