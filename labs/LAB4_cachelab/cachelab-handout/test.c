#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char** argv) {
    int M = 64;
    int N = 64;
    int A[N][M];
    int B[M][N];
    
    // initialize A and B with some value
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            A[i][j] = i*10+j;
            B[j][i] = 0;
        }
    }

    blocking_8by8_64(M, N, A, B);
    if (!is_transpose(M, N, A, B)) {
        printf("%s\n", "not a fucking transpose, dickhead");
    }
    else {
        printf("%s\n", "congradulation madafaka");
    }
}
