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
  //FIRST CASE 61x67 =========> DONE!
  int outer_column,outer_row,row,column,diagonal,tmp; 
  if(N == 67 && M == 61)
    {
      //outer column,outer row that define 8x8 block
      for (outer_column=0;outer_column<M;outer_column += 8)
        {
	  for(outer_row=0;outer_row<N;outer_row+=8)
            {
	      //now check for each index in the block 
	      //for each row in block
	      for(row=outer_row;(row<outer_row+8)&&(row<N);row++)
                {
		  //for each column in row
		  for(column=outer_column;(column<outer_column+8)&&(column<M);column++)
                    {
		      B[column][row]=A[row][column];
                    }

                }
            }
        }
    }

  //NOW GO TO 32 x 32 
  else if (N == 32 && M == 32)
    {
      for (outer_column = 0; outer_column < M; outer_column += 8) 
        { 
	  for (outer_row = 0; outer_row < N; outer_row += 8) 
            {
	      for (row = outer_row; (row < outer_row + 8)&&(row<N); row++) 
                {
		  for (column = outer_column;(column < outer_column + 8)&&(column<M);column++) 
                    {
		      if (row != column) 
                        {
			  B[column][row] = A[row][column];
                        } 
		      else 
                        {
			  //diagonal value can cause misses
			  tmp = A[row][column];
			  diagonal = row;
                        }
                    }
		  if (outer_row == outer_column) 
                    {
		      B[diagonal][diagonal] = tmp; 
                    }
                }   
            }

        }
    }

  //The hardest one
  else if (N==64 && M == 64)
    {
      int *loc;
      int offset;
      int temp0,temp1,temp2,temp3,temp4,temp5,temp6,temp7;
      for (outer_column = 0; outer_column < N; outer_column += 8)
        {
  	  for (outer_row = 0; outer_row < M; outer_row += 8)
            {

  	      temp4 = A[outer_row][outer_column+4];
  	      temp5 = A[outer_row][outer_column+5];
  	      temp6 = A[outer_row][outer_column+6];
  	      temp7 = A[outer_row][outer_column+7];

  	      for (offset = outer_row; offset < outer_row+8; offset++)
                {
  		  loc = &A[offset][outer_column];
  		  temp0 = *loc;
  		  temp1 = *(loc+1);
  		  temp2 = *(loc+2);
  		  temp3 = *(loc+3);

  		  loc = &B[outer_column][offset];
  		  *loc = temp0;
  		  *(loc+64) = temp1;
  		  *(loc+128) = temp2;
  		  *(loc+192) = temp3;
                }

  	      for (offset = outer_row+7; offset > outer_row; offset--)
                {
  		  loc = &A[offset][outer_column+4];
  		  temp0 = *loc;
  		  temp1 = *(loc+1);
  		  temp2 = *(loc+2);
  		  temp3 = *(loc+3);

  		  loc = &B[outer_column+4][offset];
  		  *loc = temp0;
  		  *(loc+64) = temp1;
  		  *(loc+128) = temp2;
  		  *(loc+192) = temp3;
                }
                
  	      loc = &B[outer_column+4][outer_row];
  	      *loc = temp4;
  	      *(loc+64) = temp5;
  	      *(loc+128) = temp6;
  	      *(loc+192) = temp7;
            }
        }
  /*   } */
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
  registerTransFunction(trans, trans_desc); 

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

