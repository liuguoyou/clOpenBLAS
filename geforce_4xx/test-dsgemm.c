#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>

int (*dgemm_gpu) (const char * transa,const char * transb,const int *m,const int *n,const int *k,const double *alpha,const double * __restrict__ a,const int *lda,const double *b,const int *ldb,const double *beta,double * c,const int *ldc );

int dgemm_gpu_impl (const char * transa,const char * transb,const int *m,const int *n,const int *k,const double *alpha,const double * __restrict__ a,const int *lda,const double *b,const int *ldb,const double *beta,double * c,const int *ldc );

extern void dgemm_ (const char * transa,const char * transb,const int *m,const int *n,const int *k,const double *alpha,const double * __restrict__ a,const int *lda,const double *b,const int *ldb,const double *beta,double * c,const int *ldc );

void * blas_gpu_info(int level, char *st, int *m, int *n, int *k);

int main (int argc, char *argv[])
{

	struct timeval tv;
	double start,end,timec,timeg;

	int lda,ldb;

	int m=2048*4;	
	int n=2048*4;
	int k=2048*4;	

	char transa='N';
	char transb='T';

	if ( transa == 'N' )
		lda = m;
	else
		lda = k;

	if ( transb == 'T' )
		ldb = n;
	else
		ldb = k;

	int ldc = m;


	unsigned long i,j;

	double alpha=1.0;
	double beta=1.0;
	double error;

	void *a1,*b1,*c1,*c2;
	double *a,*b,*cg,*cc;

	a1=malloc((size_t) 32768*8192*8);
	b1=malloc((size_t) 32768*8192*8);
	c1=malloc((size_t) 32768*32768*8);
	c2=malloc((size_t) 32768*32768*8);

	a=(double *) a1;
	b=(double *) b1;
	cg=(double *) c1;
	cc=(double *) c2;

	for( i = 0; i<m; i++)
	{
		for ( j = 0; j< k ; j++)
		{
				*a = (((double) rand() / (double) RAND_MAX) - 0.5) * 1e-1 ;
				// *a = (double) (10*i+k+1) ;
				// *a = (double) 1.0;
				a++;
		}	
	}

	for( i = 0; i<k; i++)
	{
		for( j=0; j<n; j++)
		{
			*b = (((double) rand() / (double) RAND_MAX) - 0.5) * 1e-1;
			// *b = (double) (0+1) ;
			//*b = (double) (i+k+1) ;
			b++;
		}
	}			

	// #pragma omp parallel for
	for( i = 0; i<n; i++)
	{
		for(j=0; j<m; j++)
		{
			*cc = (double) 0.0;
			*cg = (double) 0.0;
			cc++;
			cg++;
		}
	}			



	a=(double *) a1;
	b=(double *) b1;
	cg=(double *) c1;
	cc=(double *) c2;
	
	gettimeofday(&tv,NULL);
	start=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;

	int ret=0;
	int (*dsgemm_gpu)();


	dsgemm_gpu = blas_gpu_info(3, "dsgemm", NULL, NULL, NULL);
	if ( dsgemm_gpu )
	{
		ret = dsgemm_gpu(&transa,&transb,&m,&n,&k,&alpha,a,&lda,b,&ldb,&beta,cg,&ldc);
	}
	else
	{
		printf("GPU error\n");
                return(1);

	}
	if ( ret != 0)
	{
		printf("GPU error\n");
		return(1);
	}


	gettimeofday(&tv,NULL);
	end=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
	timeg=end-start;

	gettimeofday(&tv,NULL);
	start=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;

	dgemm_(&transa, &transb, &m, &n,&k ,&alpha, a, &lda, b, &ldb, &beta, cc, &ldc);

	gettimeofday(&tv,NULL);
	end=(double) tv.tv_sec+(double)tv.tv_usec*1.e-6;
	timec=end-start;


	for(i=0; i<m*n; i++)
	{
		error = fabs((cc[i] - cg[i]));
		if ( error > 0.1e-5 )
			printf("ERROR: %ld:%.16f	%.16f	%.16f\n",i,error, cg[i],cc[i]);
	}


	double fp =(2.0 * (double) m*n*k  ) * (double) 1.0e-9;
	double gflops=fp / timeg ;
	printf("GPU: %dx%dx%d size\t%10.8f sec\t%10.6f GFlop\t%10.8f GFlops\n",m,n,k,timeg,fp,gflops);
	gflops=fp / timec ;
	printf("CPU: %dx%dx%d size\t%10.8f sec\t%10.6f GFlop\t%10.8f GFlops\n",m,n,k,timec,fp,gflops);
	return(0);

}



