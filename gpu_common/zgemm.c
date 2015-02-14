/***************************************************************************
Copyright (c) 2015, The OpenBLAS Project
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.
3. Neither the name of the OpenBLAS project nor the names of
its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE OPENBLAS PROJECT OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include <omp.h>

/*
static void zgemm_print_matrix(int M, int K, double *A)
{

	int i,j;
	double *ap = A;
	for(i = 0; i<M; i++)
	{
		for(j=0; j<K; j++ )
		{
			printf("%d,%d:	%f\t%f\n", i,j,ap[0],ap[1]);
			ap+=2;
		}
	}

}

*/

/************************************* CCOPY *************************************************/
static void zgemm_gpu_ccopy(int M, int N, double *A, int LDA, double *B, blasint LDB , double *beta)
{
	int m1 = M >> 2;
	int m2 = M - m1 * 4;
	int i;

	double beta_r = beta[0];
	double beta_i = beta[1];


	if ( (beta_r == (double) 1.0) && ( beta_i == (double) 0.0 ) )
	{

	   #pragma omp parallel for num_threads(4)
	   for ( i = 0; i < N; i++)	
	   {
		double *ap = A + i*LDA*2;
		double *bp = B + i*LDB*2;

		double ar[8];

		int j;
	
		for ( j=0; j < m1 ; j++ )
		{
			ar[0] = bp[0] + ap[0];
			ar[1] = bp[1] + ap[1];
			ar[2] = bp[2] + ap[2];
			ar[3] = bp[3] + ap[3];
			ar[4] = bp[4] + ap[4];
			ar[5] = bp[5] + ap[5];
			ar[6] = bp[6] + ap[6];
			ar[7] = bp[7] + ap[7];

			bp[0] = ar[0];
			bp[1] = ar[1];
			bp[2] = ar[2];
			bp[3] = ar[3];
			bp[4] = ar[4];
			bp[5] = ar[5];
			bp[6] = ar[6];
			bp[7] = ar[7];

			ap += 8;
			bp += 8;

		}

		for ( j = 0; j < m2; j++ )
		{
			ar[0] = bp[0] + ap[0];
			ar[1] = bp[1] + ap[1];
			bp[0] = ar[0];
			bp[1] = ar[1];
			ap += 2;
			bp += 2;

		}
	
	   }



	}
	else
	{

	   #pragma omp parallel for num_threads(4)
	   for ( i = 0; i < N; i++)	
	   {
		double *ap = A + i*LDA*2;
		double *bp = B + i*LDB*2;

		double ar[8];

		int j;
	
		for ( j=0; j < m1 ; j++ )
		{
			ar[0] = bp[0] * beta_r - bp[1] * beta_i;
			ar[1] = bp[1] * beta_r + bp[0] * beta_i;

			ar[2] = bp[2] * beta_r - bp[3] * beta_i;
			ar[3] = bp[3] * beta_r + bp[2] * beta_i;

			ar[4] = bp[4] * beta_r - bp[5] * beta_i;
			ar[5] = bp[5] * beta_r + bp[4] * beta_i;

			ar[6] = bp[6] * beta_r - bp[7] * beta_i;
			ar[7] = bp[7] * beta_r + bp[6] * beta_i;

			bp[0] = ar[0] + ap[0];
			bp[1] = ar[1] + ap[1];
			bp[2] = ar[2] + ap[2];
			bp[3] = ar[3] + ap[3];
			bp[4] = ar[4] + ap[4];
			bp[5] = ar[5] + ap[5];
			bp[6] = ar[6] + ap[6];
			bp[7] = ar[7] + ap[7];

			ap += 8;
			bp += 8;

		}

		for ( j = 0; j < m2; j++ )
		{
			ar[0] = bp[0] * beta_r - bp[1] * beta_i;
			ar[1] = bp[1] * beta_r + bp[0] * beta_i;
			bp[0] = ar[0] + ap[0];
			bp[1] = ar[1] + ap[1];
			ap += 2;
			bp += 2;

		}
	
	   }

	}
	return;


}



/************************************* BCOPY *************************************************/

static void zgemm_gpu_bcopy(int M, int N, double *A , blasint LDA, double *B, int PAD_M, int PAD_N)
{
	int i;

	int pad_m = (( M + PAD_M - 1 ) & -PAD_M) - M;
	int pad_n = (( N + PAD_N - 1 ) & -PAD_N) - N;

	int n1 = N/4;
	int n2 = N - n1*4;

	#pragma omp parallel for num_threads(3)
	for ( i=0; i < M ; i++ )
	{

		double *a_ptr = A + i * LDA *2;
		double *b_ptr = B + i * (N + pad_n)*2;
		
		double *ap = a_ptr;
		double *bp = b_ptr;
		int j;
		double ar[8];

		for ( j=0; j < n1 ; j++ )
		{

			ar[0] = *(ap + 0);
			ar[1] = *(ap + 1);
			ar[2] = *(ap + 2);
			ar[3] = *(ap + 3);
			ar[4] = *(ap + 4);
			ar[5] = *(ap + 5);
			ar[6] = *(ap + 6);
			ar[7] = *(ap + 7);

			*(bp + 0) = ar[0];
			*(bp + 1) = ar[1];
			*(bp + 2) = ar[2];
			*(bp + 3) = ar[3];
			*(bp + 4) = ar[4];
			*(bp + 5) = ar[5];
			*(bp + 6) = ar[6];
			*(bp + 7) = ar[7];

			ap += 8;
			bp += 8;

		}

		for (j=0 ; j < n2 ; j++ )
		{

			ar[0] = *(ap + 0);
			ar[1] = *(ap + 1);
			*(bp + 0) = ar[0];
			*(bp + 1) = ar[1];
			ap += 2;
			bp += 2;
		}

		for ( ; j < n2 + pad_n; j++ )
		{
			*(bp + 0) = (double) 0.0;
			*(bp + 1) = (double) 0.0;
			bp += 2;

		}
		
	}

	for ( i=M; i < M + pad_m; i++)
	{
		double *b_ptr = B + i * (N + pad_n) *2;
		double *bp = b_ptr;

		int j;
		for ( j=0; j < N + pad_n; j++ )
                {

			*(bp + 0) = (double) 0.0;
			*(bp + 1) = (double) 0.0;
			bp += 2;
                }
	}

}

/************************************* BCOPY CONJ*********************************************/

static void zgemm_gpu_bcopy_conj(int M, int N, double *A , blasint LDA, double *B, int PAD_M, int PAD_N)
{
	int i;

	int pad_m = (( M + PAD_M - 1 ) & -PAD_M) - M;
	int pad_n = (( N + PAD_N - 1 ) & -PAD_N) - N;

	int n1 = N/4;
	int n2 = N - n1*4;

	#pragma omp parallel for num_threads(3)
	for ( i=0; i < M ; i++ )
	{

		double *a_ptr = A + i * LDA *2;
		double *b_ptr = B + i * (N + pad_n)*2;
		
		double *ap = a_ptr;
		double *bp = b_ptr;
		int j;
		double ar[8];

		for ( j=0; j < n1 ; j++ )
		{

			ar[0] = *(ap + 0);
			ar[1] = *(ap + 1);
			ar[2] = *(ap + 2);
			ar[3] = *(ap + 3);
			ar[4] = *(ap + 4);
			ar[5] = *(ap + 5);
			ar[6] = *(ap + 6);
			ar[7] = *(ap + 7);

			*(bp + 0) = ar[0];
			*(bp + 1) = -ar[1];
			*(bp + 2) = ar[2];
			*(bp + 3) = -ar[3];
			*(bp + 4) = ar[4];
			*(bp + 5) = -ar[5];
			*(bp + 6) = ar[6];
			*(bp + 7) = -ar[7];

			ap += 8;
			bp += 8;

		}

		for (j=0 ; j < n2 ; j++ )
		{

			ar[0] = *(ap + 0);
			ar[1] = *(ap + 1);
			*(bp + 0) = ar[0];
			*(bp + 1) = -ar[1];
			ap += 2;
			bp += 2;
		}

		for ( ; j < n2 + pad_n; j++ )
		{
			*(bp + 0) = (double) 0.0;
			*(bp + 1) = (double) 0.0;
			bp += 2;

		}
		
	}

	for ( i=M; i < M + pad_m; i++)
	{
		double *b_ptr = B + i * (N + pad_n) *2;
		double *bp = b_ptr;

		int j;
		for ( j=0; j < N + pad_n; j++ )
                {

			*(bp + 0) = (double) 0.0;
			*(bp + 1) = (double) 0.0;
			bp += 2;
                }
	}

}




/************************************* ACOPY *************************************************/

static void zgemm_gpu_acopy(int M, int K, double *A , blasint LDA, double *B, int PAD_M, int PAD_K)
{
	int i;

	int pad_m = (( M + PAD_M - 1 ) & -PAD_M) - M;
	int pad_k = (( K + PAD_K - 1 ) & -PAD_K) - K;


	// double *a_ptr = A;
	// double *b_ptr = B;

	int m1 = M >> 3;
	int m2 = M - m1 * 8;

	#pragma omp parallel for num_threads(3)
	for ( i=0; i < K ; i++ )
	{

		// printf("OMP Thread %d\n", omp_get_thread_num());
		double *a_ptr = A + i * LDA*2;
		double *b_ptr = B + i * (M + pad_m)*2;
		
		double *ap = a_ptr;
		double *bp = b_ptr;
		int j;
		double ar[16];

		for ( j=0; j < m1 ; j++ )
		{

			ar[0] = *(ap + 0);
			ar[1] = *(ap + 1);
			ar[2] = *(ap + 2);
			ar[3] = *(ap + 3);
			ar[4] = *(ap + 4);
			ar[5] = *(ap + 5);
			ar[6] = *(ap + 6);
			ar[7] = *(ap + 7);
			ar[8] = *(ap + 8);
			ar[9] = *(ap + 9);
			ar[10] = *(ap + 10);
			ar[11] = *(ap + 11);
			ar[12] = *(ap + 12);
			ar[13] = *(ap + 13);
			ar[14] = *(ap + 14);
			ar[15] = *(ap + 15);

			*(bp + 0) = ar[0];
			*(bp + 1) = ar[1];
			*(bp + 2) = ar[2];
			*(bp + 3) = ar[3];
			*(bp + 4) = ar[4];
			*(bp + 5) = ar[5];
			*(bp + 6) = ar[6];
			*(bp + 7) = ar[7];
			*(bp + 8) = ar[8];
			*(bp + 9) = ar[9];
			*(bp + 10) = ar[10];
			*(bp + 11) = ar[11];
			*(bp + 12) = ar[12];
			*(bp + 13) = ar[13];
			*(bp + 14) = ar[14];
			*(bp + 15) = ar[15];

			ap += 16;
			bp += 16;

		}

		for (j=0 ; j < m2 ; j++ )
		{
			bp[0] = ap[0];
			bp[1] = ap[1];

			ap += 2;
			bp += 2;
		}

		for ( ; j < m2 + pad_m; j++ )
		{

			bp[0] = (double) 0.0;
			bp[1] = (double) 0.0;
			bp += 2;
		}
		
	}

	for ( i=K; i < K + pad_k; i++)
	{
		double *b_ptr = B + i * (M + pad_m)*2;
		double *bp = b_ptr;

		int j;
		for ( j=0; j < M + pad_m; j++ )
                {
			bp[0] = (double) 0.0;
			bp[1] = (double) 0.0;
			bp += 2;
                }
	}

}

static void zgemm_gpu_btcopy(int M, int K, double *A , blasint LDA, double *B, int PAD_M, int PAD_K)
{
	unsigned long i=0;

	int pad_m = (( M + PAD_M - 1 ) & -PAD_M) - M;

	int mp = (M + pad_m) *2;


	double *a_ptr = A;
	double *b_ptr = B;

	// #pragma omp parallel for num_threads(3)
	for ( i=0; i < (M/4)*4 ; i+=4 )
	{

		// double *a_ptr = A + i * (unsigned long) LDA;
		// double *b_ptr = B + i * 4;
	
		
		double *ap = a_ptr;
		double *ap1 = ap  + LDA*2;
		double *ap2 = ap1 + LDA*2;
		double *ap3 = ap2 + LDA*2;

		double *bp = b_ptr;
		double *bp1 = b_ptr +2;
		double *bp2 = b_ptr +4;
		double *bp3 = b_ptr +6;

		int j;
		double ar[16];
		
		int mp0 = 0;
		int mp1 = mp;
		int mp2 = mp1+mp;
		int mp3 = mp2+mp;

		for (j=0 ; j < (K/4)*4 ; j+=4 )
		{

			ar[0] = *(ap);  
			ar[1] = *(ap+1);  
			ar[2] = *(ap+2);  
			ar[3] = *(ap+3);  

			ar[4] = *(ap+4);
			ar[5] = *(ap+5);
			ar[6] = *(ap+6);
			ar[7] = *(ap+7);

			ar[8]  = *(ap1);
			ar[9]  = *(ap1+1);
			ar[10] = *(ap1+2);
			ar[11] = *(ap1+3);

			ar[12] = *(ap1+4);
			ar[13] = *(ap1+5);
			ar[14] = *(ap1+6);
			ar[15] = *(ap1+7);

			*(bp + mp0)     = ar[0];
			*(bp + mp0 +1 ) = ar[1];
			*(bp + mp1)     = ar[2];
			*(bp + mp1 +1 ) = ar[3];
			*(bp + mp2)     = ar[4];
			*(bp + mp2 +1 ) = ar[5];
			*(bp + mp3)     = ar[6];
			*(bp + mp3 +1 ) = ar[7];

			*(bp1 + mp0)     = ar[8];
			*(bp1 + mp0 +1 ) = ar[9];
			*(bp1 + mp1)     = ar[10];
			*(bp1 + mp1 +1 ) = ar[11];
			*(bp1 + mp2)     = ar[12];
			*(bp1 + mp2 +1 ) = ar[13];
			*(bp1 + mp3)     = ar[14];
			*(bp1 + mp3 +1 ) = ar[15];


			ar[0] = *(ap2);  
			ar[1] = *(ap2+1);  
			ar[2] = *(ap2+2);  
			ar[3] = *(ap2+3);  

			ar[4] = *(ap2+4);
			ar[5] = *(ap2+5);
			ar[6] = *(ap2+6);
			ar[7] = *(ap2+7);

			ar[8]  = *(ap3);
			ar[9]  = *(ap3+1);
			ar[10] = *(ap3+2);
			ar[11] = *(ap3+3);

			ar[12] = *(ap3+4);
			ar[13] = *(ap3+5);
			ar[14] = *(ap3+6);
			ar[15] = *(ap3+7);

			*(bp2 + mp0)     = ar[0];
			*(bp2 + mp0 +1 ) = ar[1];
			*(bp2 + mp1)     = ar[2];
			*(bp2 + mp1 +1 ) = ar[3];
			*(bp2 + mp2)     = ar[4];
			*(bp2 + mp2 +1 ) = ar[5];
			*(bp2 + mp3)     = ar[6];
			*(bp2 + mp3 +1 ) = ar[7];

			*(bp3 + mp0)     = ar[8];
			*(bp3 + mp0 +1 ) = ar[9];
			*(bp3 + mp1)     = ar[10];
			*(bp3 + mp1 +1 ) = ar[11];
			*(bp3 + mp2)     = ar[12];
			*(bp3 + mp2 +1 ) = ar[13];
			*(bp3 + mp3)     = ar[14];
			*(bp3 + mp3 +1 ) = ar[15];

			mp0 += 4 * mp;
			mp1 = mp0 + mp;
			mp2 = mp1 + mp;
			mp3 = mp2 + mp;
			ap  += 8;
			ap1 += 8;
			ap2 += 8;
			ap3 += 8;
		}

		for ( ; j < K ; j++ )
		{

			ar[0]  = *(ap);  
			ar[1]  = *(ap +1 );  
			ar[4]  = *(ap1);  
			ar[5]  = *(ap1 +1);  
			ar[8]  = *(ap2);  
			ar[9]  = *(ap2 +1);  
			ar[12] = *(ap3);  
			ar[13] = *(ap3 +1);  

			*(bp + mp0 )        = ar[0];
			*(bp + mp0 +1)      = ar[1];
			*(bp + mp0 +2)      = ar[4];
			*(bp + mp0 +3)      = ar[5];
			*(bp + mp0 +4)      = ar[8];
			*(bp + mp0 +5)      = ar[9];
			*(bp + mp0 +6)      = ar[12];
			*(bp + mp0 +7)      = ar[13];

			mp0 += mp;
			ap += 2;
			ap1+= 2;
			ap2+= 2;
			ap3+= 2;

		}

		a_ptr += 4*LDA*2;
		b_ptr += 8;
	}

	for ( ; i < M ; i++ )
	{

		double *ap = a_ptr;
		double *bp = b_ptr;
		int j;
		double ar[8];
		
		int mp0 = 0;
		int mp1 = mp;
		int mp2 = mp1+mp;
		int mp3 = mp2+mp;

		for (j=0 ; j < (K/4)*4 ; j+=4 )
		{

			ar[0] = *(ap);  
			ar[1] = *(ap+1);  
			ar[2] = *(ap+2);  
			ar[3] = *(ap+3);  
			ar[4] = *(ap+4);  
			ar[5] = *(ap+5);  
			ar[6] = *(ap+6);  
			ar[7] = *(ap+7);  

			*(bp  + mp0) 	 = ar[0];
			*(bp  + mp0 +1 ) = ar[1];
			*(bp  + mp1)     = ar[2];
			*(bp  + mp1 +1)  = ar[3];
			*(bp  + mp2)     = ar[4];
			*(bp  + mp2 +1)  = ar[5];
			*(bp  + mp3)     = ar[6];
			*(bp  + mp3 +1)  = ar[7];

			mp0 += 4 * mp;
			mp1 = mp0 + mp;
			mp2 = mp1 + mp;
			mp3 = mp2 + mp;
			ap  += 8;
		}

		for ( ; j < K ; j++ )
		{

			ar[0] = *(ap);  
			ar[1] = *(ap +1);  

			*(bp + mp0 )        = ar[0];
			*(bp + mp0 +1)      = ar[1];

			mp0 += mp;
			ap++;
		}

		a_ptr += 2*LDA;
		b_ptr += 2;
	}

}


static void zgemm_gpu_btcopy_conj(int M, int K, double *A , blasint LDA, double *B, int PAD_M, int PAD_K)
{
	unsigned long i=0;

	int pad_m = (( M + PAD_M - 1 ) & -PAD_M) - M;

	int mp = (M + pad_m) *2;


	double *a_ptr = A;
	double *b_ptr = B;

	// #pragma omp parallel for num_threads(3)
	for ( i=0; i < (M/4)*4 ; i+=4 )
	{

		// double *a_ptr = A + i * (unsigned long) LDA;
		// double *b_ptr = B + i * 4;
	
		
		double *ap = a_ptr;
		double *ap1 = ap  + LDA*2;
		double *ap2 = ap1 + LDA*2;
		double *ap3 = ap2 + LDA*2;

		double *bp = b_ptr;
		double *bp1 = b_ptr +2;
		double *bp2 = b_ptr +4;
		double *bp3 = b_ptr +6;

		int j;
		double ar[16];
		
		int mp0 = 0;
		int mp1 = mp;
		int mp2 = mp1+mp;
		int mp3 = mp2+mp;

		for (j=0 ; j < (K/4)*4 ; j+=4 )
		{

			ar[0] = *(ap);  
			ar[1] = *(ap+1);  
			ar[2] = *(ap+2);  
			ar[3] = *(ap+3);  

			ar[4] = *(ap+4);
			ar[5] = *(ap+5);
			ar[6] = *(ap+6);
			ar[7] = *(ap+7);

			ar[8]  = *(ap1);
			ar[9]  = *(ap1+1);
			ar[10] = *(ap1+2);
			ar[11] = *(ap1+3);

			ar[12] = *(ap1+4);
			ar[13] = *(ap1+5);
			ar[14] = *(ap1+6);
			ar[15] = *(ap1+7);

			*(bp + mp0)     = ar[0];
			*(bp + mp0 +1 ) = -ar[1];
			*(bp + mp1)     = ar[2];
			*(bp + mp1 +1 ) = -ar[3];
			*(bp + mp2)     = ar[4];
			*(bp + mp2 +1 ) = -ar[5];
			*(bp + mp3)     = ar[6];
			*(bp + mp3 +1 ) = -ar[7];

			*(bp1 + mp0)     = ar[8];
			*(bp1 + mp0 +1 ) = -ar[9];
			*(bp1 + mp1)     = ar[10];
			*(bp1 + mp1 +1 ) = -ar[11];
			*(bp1 + mp2)     = ar[12];
			*(bp1 + mp2 +1 ) = -ar[13];
			*(bp1 + mp3)     = ar[14];
			*(bp1 + mp3 +1 ) = -ar[15];


			ar[0] = *(ap2);  
			ar[1] = *(ap2+1);  
			ar[2] = *(ap2+2);  
			ar[3] = *(ap2+3);  

			ar[4] = *(ap2+4);
			ar[5] = *(ap2+5);
			ar[6] = *(ap2+6);
			ar[7] = *(ap2+7);

			ar[8]  = *(ap3);
			ar[9]  = *(ap3+1);
			ar[10] = *(ap3+2);
			ar[11] = *(ap3+3);

			ar[12] = *(ap3+4);
			ar[13] = *(ap3+5);
			ar[14] = *(ap3+6);
			ar[15] = *(ap3+7);

			*(bp2 + mp0)     = ar[0];
			*(bp2 + mp0 +1 ) = -ar[1];
			*(bp2 + mp1)     = ar[2];
			*(bp2 + mp1 +1 ) = -ar[3];
			*(bp2 + mp2)     = ar[4];
			*(bp2 + mp2 +1 ) = -ar[5];
			*(bp2 + mp3)     = ar[6];
			*(bp2 + mp3 +1 ) = -ar[7];

			*(bp3 + mp0)     = ar[8];
			*(bp3 + mp0 +1 ) = -ar[9];
			*(bp3 + mp1)     = ar[10];
			*(bp3 + mp1 +1 ) = -ar[11];
			*(bp3 + mp2)     = ar[12];
			*(bp3 + mp2 +1 ) = -ar[13];
			*(bp3 + mp3)     = ar[14];
			*(bp3 + mp3 +1 ) = -ar[15];

			mp0 += 4 * mp;
			mp1 = mp0 + mp;
			mp2 = mp1 + mp;
			mp3 = mp2 + mp;
			ap  += 8;
			ap1 += 8;
			ap2 += 8;
			ap3 += 8;
		}

		for ( ; j < K ; j++ )
		{

			ar[0]  = *(ap);  
			ar[1]  = *(ap +1 );  
			ar[4]  = *(ap1);  
			ar[5]  = *(ap1 +1);  
			ar[8]  = *(ap2);  
			ar[9]  = *(ap2 +1);  
			ar[12] = *(ap3);  
			ar[13] = *(ap3 +1);  

			*(bp + mp0 )        = ar[0];
			*(bp + mp0 +1)      = -ar[1];
			*(bp + mp0 +2)      = ar[4];
			*(bp + mp0 +3)      = -ar[5];
			*(bp + mp0 +4)      = ar[8];
			*(bp + mp0 +5)      = -ar[9];
			*(bp + mp0 +6)      = ar[12];
			*(bp + mp0 +7)      = -ar[13];

			mp0 += mp;
			ap += 2;
			ap1+= 2;
			ap2+= 2;
			ap3+= 2;

		}

		a_ptr += 4*LDA*2;
		b_ptr += 8;
	}

	for ( ; i < M ; i++ )
	{

		double *ap = a_ptr;
		double *bp = b_ptr;
		int j;
		double ar[8];
		
		int mp0 = 0;
		int mp1 = mp;
		int mp2 = mp1+mp;
		int mp3 = mp2+mp;

		for (j=0 ; j < (K/4)*4 ; j+=4 )
		{

			ar[0] = *(ap);  
			ar[1] = *(ap+1);  
			ar[2] = *(ap+2);  
			ar[3] = *(ap+3);  
			ar[4] = *(ap+4);  
			ar[5] = *(ap+5);  
			ar[6] = *(ap+6);  
			ar[7] = *(ap+7);  

			*(bp  + mp0) 	 = ar[0];
			*(bp  + mp0 +1 ) = -ar[1];
			*(bp  + mp1)     = ar[2];
			*(bp  + mp1 +1)  = -ar[3];
			*(bp  + mp2)     = ar[4];
			*(bp  + mp2 +1)  = -ar[5];
			*(bp  + mp3)     = ar[6];
			*(bp  + mp3 +1)  = -ar[7];

			mp0 += 4 * mp;
			mp1 = mp0 + mp;
			mp2 = mp1 + mp;
			mp3 = mp2 + mp;
			ap  += 8;
		}

		for ( ; j < K ; j++ )
		{

			ar[0] = *(ap);  
			ar[1] = *(ap +1);  

			*(bp + mp0 )        = ar[0];
			*(bp + mp0 +1)      = -ar[1];

			mp0 += mp;
			ap++;
		}

		a_ptr += 2*LDA;
		b_ptr += 2;
	}

}





static int zgemm_gpu_simple(char *TRANSA, char *TRANSB, blasint *M, blasint *N, blasint *K, double *ALPHA, double *A, blasint *LDA, double *B, blasint *LDB, double *BETA, double *C, blasint *LDC)
{

	int ret;

	#ifdef PROFILE
        	struct timeval tv;
        	double start,end,time;
	#endif

	double ktime = 0.0;

	if ( have_gpu_context == 0 )
	{
		printf("GPU Error\n");
		return(1);
	}

	#ifdef PROFILE
		printf("BEGIN create program\n");
        	gettimeofday(&tv,NULL);
        	start=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
	#endif

        ret = create_gpu_program_nonunified(&gpu, "gemm",  (size_t) ALLOC_SIZE);
        if ( ret )
	{
		have_gpu_context = 0;
		printf("GPU Error\n");
                return(1);

	}

	#ifdef PROFILE
        	gettimeofday(&tv,NULL);
        	end=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
		time=end-start;
		printf("END create program\n");
		printf("OpenCL create program summary:\t\t%f sec\n", time);
	#endif

	int info = 0;
	int transa = -1;
	int transb = -1;
	char transA = toupper(*TRANSA);
	char transB = toupper(*TRANSB);

	if ( transA == 'N' ) transa = 0;
	if ( transA == 'T' ) transa = 1;
	if ( transA == 'C' ) transa = 2;
	if ( transB == 'N' ) transb = 0;
	if ( transB == 'T' ) transb = 1;
	if ( transB == 'C' ) transb = 2;

	if ( transb < 0 ) info = 2;
	if ( transa < 0 ) info = 1;

	if ( info > 0 )
	{
		printf("** On entry to zgemm_ parameter number %d had a illegal value\n",info);
		return(2);
	}

	if ( *LDC < *M ) info = 13;
	if ((( transb > 0 ) && ( *LDB < *N )) || (( transb == 0 ) &&  ( *LDB < *K ))) info = 10;
	if ((( transa > 0 ) && ( *LDA < *K )) || (( transa == 0 ) &&  ( *LDA < *M ))) info = 8;
	if ( *K < 1 ) info = 5;
	if ( *N < 1 ) info = 4;
	if ( *M < 1 ) info = 3;

	if ( info > 0 )
	{
		printf("** On entry to zgemm_ parameter number %d had a illegal value\n",info);
		return(2);
	}


	void *bcopy_ptr[ZGEMM_N_BUFFERS + 1];
	int i1;
	for (i1 = 0; i1 < ZGEMM_N_BUFFERS+1; i1++)
		bcopy_ptr[i1] = gpu.hB + i1 * GALLOC_SIZE_B;

	int ZGEMM_M_MAX_RUN;
	int ZGEMM_N_MAX_RUN;
	int ZGEMM_K_MAX_RUN = ZGEMM_K_MAX;


	if ( transA == 'N' )
		ZGEMM_M_MAX_RUN = ZGEMM_M_MAX;
	else
		ZGEMM_M_MAX_RUN = 768;
	
	if ( transB == 'N' )
		ZGEMM_N_MAX_RUN = 768;
	else
		ZGEMM_N_MAX_RUN = ZGEMM_N_MAX;
	

	int bcopy = 1;
	int acopy = 1;

	#ifdef PROFILE
		ktime = 0.0;
	#endif

	double beta[2];
	beta[0] = BETA[0];
	beta[1] = BETA[1];

	int zgemm_m_run;
	int zgemm_n_run;
	int zgemm_k_run;

	int zgemm_k = ZGEMM_K_MAX_RUN;
	int l=0;

	while ( zgemm_k == ZGEMM_K_MAX_RUN )
	{
		
		int saved_bcopy=-1;

		if ( l * ZGEMM_K_MAX_RUN + zgemm_k > *K )
		{
			zgemm_k = *K % ZGEMM_K_MAX_RUN;
			if ( zgemm_k == 0 )
			{
				break;
			}
		}

		zgemm_k_run = ( zgemm_k + ZGEMM_PAD_K - 1 ) & -ZGEMM_PAD_K;

		int zgemm_m = ZGEMM_M_MAX_RUN;
		int i=0;

		while ( zgemm_m == ZGEMM_M_MAX_RUN )
		{
			if ( i * ZGEMM_M_MAX_RUN + zgemm_m > *M )
			{
				zgemm_m = *M % ZGEMM_M_MAX_RUN;
				if ( zgemm_m == 0 )
				{
					break;
				}	
			}

			zgemm_m_run = ( zgemm_m + ZGEMM_PAD_M - 1 ) & -ZGEMM_PAD_M;

			double *a_ptr;

			if ( transA == 'N' )
			{
				a_ptr = A + ( l* ZGEMM_K_MAX_RUN * *LDA + i * ZGEMM_M_MAX_RUN ) *2 ;

				#ifdef PROFILE
					printf("----------------------------------------------------------------------------------\n");
        				gettimeofday(&tv,NULL);
        				start=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
				#endif

				zgemm_gpu_acopy(zgemm_m, zgemm_k, a_ptr, *LDA, (double*) gpu.hA, ZGEMM_PAD_M, ZGEMM_PAD_K);
				// zgemm_print_matrix(zgemm_m_run, zgemm_k_run, gpu.hA);

				#ifdef PROFILE
        				gettimeofday(&tv,NULL);
        				end=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
					time=end-start;
					printf("Zgemm acopy:\t\t\t\t%f sec\n", time);
				#endif

				#ifdef PROFILE
        				gettimeofday(&tv,NULL);
        				start=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
				#endif

				clEnqueueWriteBuffer(gpu.command_queue, gpu.A, CL_FALSE, 0, zgemm_m_run * zgemm_k_run *sizeof(double) *2, gpu.hA, 0, NULL, NULL);

				#ifdef PROFILE
        				gettimeofday(&tv,NULL);
        				end=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
					time=end-start;
					printf("Prof: enqueue A:\t\t\t%f sec\n", time);
				#endif

			}
			else
			{
				a_ptr = A + ( i * ZGEMM_M_MAX_RUN * *LDA + l * ZGEMM_K_MAX_RUN ) *2;

				#ifdef PROFILE
					printf("----------------------------------------------------------------------------------\n");
        				gettimeofday(&tv,NULL);
        				start=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
				#endif

				memset(gpu.hA, 0, (size_t) zgemm_m_run * zgemm_k_run * sizeof(double));
				if ( transa == 2 )
					zgemm_gpu_btcopy_conj(zgemm_m, zgemm_k, a_ptr, *LDA, (double*) gpu.hA, ZGEMM_PAD_M, ZGEMM_PAD_K);

				else
					zgemm_gpu_btcopy(zgemm_m, zgemm_k, a_ptr, *LDA, (double*) gpu.hA, ZGEMM_PAD_M, ZGEMM_PAD_K);

				#ifdef PROFILE
        				gettimeofday(&tv,NULL);
        				end=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
					time=end-start;
					printf("Zgemm atcopy:\t\t\t\t%f sec\n", time);
				#endif

				#ifdef PROFILE
        				gettimeofday(&tv,NULL);
        				start=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
				#endif

				clEnqueueWriteBuffer(gpu.command_queue, gpu.A, CL_FALSE, 0, zgemm_m_run * zgemm_k_run *sizeof(double) *2, gpu.hA, 0, NULL, NULL);

				#ifdef PROFILE
        				gettimeofday(&tv,NULL);
        				end=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
					time=end-start;
					printf("Prof: enqueue A:\t\t\t%f sec\n", time);
				#endif


			}

			int zgemm_n = ZGEMM_N_MAX_RUN;
			int j=0;
			acopy = 1;

			int curr_bcopy = 0;

			while ( zgemm_n == ZGEMM_N_MAX_RUN )
			{

				if ( j * ZGEMM_N_MAX_RUN + zgemm_n > *N )
				{
					zgemm_n = *N % ZGEMM_N_MAX_RUN;
					if ( zgemm_n == 0 )
					{
						break;
					}
				}

				zgemm_n_run = ( zgemm_n + ZGEMM_PAD_N - 1 ) & -ZGEMM_PAD_N;

				double *c_ptr = C + ( j * ZGEMM_N_MAX_RUN * *LDC + i * ZGEMM_M_MAX_RUN ) *2; 
				double *b_ptr;

				if ( transB == 'N' )
				{
					b_ptr = B + ( j* ZGEMM_N_MAX_RUN * *LDB + l*ZGEMM_K_MAX_RUN ) *2;

					if ( curr_bcopy > saved_bcopy )
					{

						#ifdef PROFILE
        						gettimeofday(&tv,NULL);
        						start=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
						#endif

						if ( curr_bcopy < ZGEMM_N_BUFFERS )
						{
							memset(bcopy_ptr[curr_bcopy], 0, (size_t) zgemm_n_run * zgemm_k_run * sizeof(double));
							zgemm_gpu_btcopy(zgemm_n, zgemm_k, b_ptr, *LDB, (double*) bcopy_ptr[curr_bcopy], ZGEMM_PAD_N, ZGEMM_PAD_K);
							saved_bcopy++;
						}
						else
						{
							memset(bcopy_ptr[ZGEMM_N_BUFFERS], 0, (size_t) zgemm_n_run * zgemm_k_run * sizeof(double));
							zgemm_gpu_btcopy(zgemm_n, zgemm_k, b_ptr, *LDB, (double*) bcopy_ptr[ZGEMM_N_BUFFERS], ZGEMM_PAD_N, ZGEMM_PAD_K);

						}

						#ifdef PROFILE
        						gettimeofday(&tv,NULL);
        						end=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
							time=end-start;
							printf("Zgemm btcopy:\t\t\t\t%f sec\n", time);
						#endif
					}

					#ifdef PROFILE
        					gettimeofday(&tv,NULL);
        					start=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
					#endif

					if ( curr_bcopy < ZGEMM_N_BUFFERS )
						clEnqueueWriteBuffer(gpu.command_queue, gpu.B, CL_FALSE, 0, zgemm_n_run * zgemm_k_run *sizeof(double) *2, bcopy_ptr[curr_bcopy], 0, NULL, NULL);
					else
						clEnqueueWriteBuffer(gpu.command_queue, gpu.B, CL_FALSE, 0, zgemm_n_run * zgemm_k_run *sizeof(double) *2, bcopy_ptr[ZGEMM_N_BUFFERS], 0, NULL, NULL);

					#ifdef PROFILE
        					gettimeofday(&tv,NULL);
        					end=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
						time=end-start;
						printf("Prof: enqueue B:\t\t\t%f sec\n", time);
					#endif



				}
				else
				{
					b_ptr = B + ( l* ZGEMM_K_MAX_RUN * *LDB + j*ZGEMM_N_MAX_RUN ) *2;

					if ( curr_bcopy > saved_bcopy )
					{

						#ifdef PROFILE
        						gettimeofday(&tv,NULL);
        						start=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
						#endif

						if ( curr_bcopy < ZGEMM_N_BUFFERS )
						{
							if ( transb == 2 )
								zgemm_gpu_bcopy_conj(zgemm_k, zgemm_n, b_ptr, *LDB, (double*) bcopy_ptr[curr_bcopy], ZGEMM_PAD_K, ZGEMM_PAD_N);
							else
								zgemm_gpu_bcopy(zgemm_k, zgemm_n, b_ptr, *LDB, (double*) bcopy_ptr[curr_bcopy], ZGEMM_PAD_K, ZGEMM_PAD_N);
							//
							// zgemm_print_matrix(zgemm_n_run, zgemm_k_run, bcopy_ptr[curr_bcopy]);
							saved_bcopy++;
						}
						else
						{
							if ( transb == 2 )
								zgemm_gpu_bcopy_conj(zgemm_k, zgemm_n, b_ptr, *LDB, (double*) bcopy_ptr[ZGEMM_N_BUFFERS], ZGEMM_PAD_K, ZGEMM_PAD_N);
							else
								zgemm_gpu_bcopy(zgemm_k, zgemm_n, b_ptr, *LDB, (double*) bcopy_ptr[ZGEMM_N_BUFFERS], ZGEMM_PAD_K, ZGEMM_PAD_N);

						}

						#ifdef PROFILE
        						gettimeofday(&tv,NULL);
        						end=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
							time=end-start;
							printf("Zgemm bcopy:\t\t\t\t%f sec\n", time);
						#endif
					}

					#ifdef PROFILE
        					gettimeofday(&tv,NULL);
        					start=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
					#endif

					if ( curr_bcopy < ZGEMM_N_BUFFERS )
						clEnqueueWriteBuffer(gpu.command_queue, gpu.B, CL_FALSE, 0, zgemm_n_run * zgemm_k_run *sizeof(double)*2, bcopy_ptr[curr_bcopy], 0, NULL, NULL);
					else
						clEnqueueWriteBuffer(gpu.command_queue, gpu.B, CL_FALSE, 0, zgemm_n_run * zgemm_k_run *sizeof(double)*2, bcopy_ptr[ZGEMM_N_BUFFERS], 0, NULL, NULL);

					#ifdef PROFILE
       						gettimeofday(&tv,NULL);
       						end=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
						time=end-start;
						printf("Prof: enqueue B:\t\t\t%f sec\n", time);
					#endif


				}

				// printf("ACOPY: %d	BCOPY: %d\n", acopy,bcopy);
				ret = zgemm_gpu_kernel(&gpu, zgemm_m_run, zgemm_n_run, zgemm_k_run, ALPHA, acopy, bcopy, &ktime);
				if ( ret != CL_SUCCESS )
				{
					
					printf("GPU Error\n");
					release_gpu_program(&gpu);
					return(1);
				}

				#ifdef PROFILE
        				gettimeofday(&tv,NULL);
        				start=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
				#endif
				// zgemm_print_matrix(zgemm_m_run, zgemm_n_run, gpu.hC);
				zgemm_gpu_ccopy(zgemm_m, zgemm_n, (double*) gpu.hC, zgemm_m_run , c_ptr, *LDC, beta);
				#ifdef PROFILE
        				gettimeofday(&tv,NULL);
        				end=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
					time=end-start;
					printf("Zgemm ccopy:\t\t\t\t%f sec\n\n", time);
				#endif


				acopy = 0;
				j++;
				curr_bcopy++;

			}
			i++;

		}
		beta[0] = (double) 1.0;
		beta[1] = (double) 0.0;
		l++;
	}

	#ifdef PROFILE
		printf("----------------------------------------------------------------------------------\n");
		printf("OpenCL Kernel time sum:\t\t\t%f sec\n", ktime);
		printf("OpenCL GFLops:\t\t\t\t%f\n", (double) *K * (double) *M * (double) *N * (double) 8.0 / ktime  * (double) 1e-9);
		printf("----------------------------------------------------------------------------------\n");
	#endif
	release_gpu_program(&gpu);
	// destroy_gpu_context(&gpu);
	return(0);

}

