#define ALLOC_SIZE 4096*4096*4

#define HAVE_DSGEMM 1

#define SGEMM_M_MAX 4096
#define SGEMM_N_MAX 4096
#define SGEMM_K_MAX 4096

#define DGEMM_M_MAX 2048
#define DGEMM_N_MAX 2048
#define DGEMM_K_MAX 4096

#define CGEMM_M_MAX 2048
#define CGEMM_N_MAX 2048
#define CGEMM_K_MAX 4096

#define ZGEMM_M_MAX 1024
#define ZGEMM_N_MAX 1024
#define ZGEMM_K_MAX 4096

#define SGEMM_N_BUFFERS 16
#define DGEMM_N_BUFFERS 16
#define CGEMM_N_BUFFERS 16
#define ZGEMM_N_BUFFERS 16

#define GALLOC_SIZE_A ( SGEMM_M_MAX * SGEMM_K_MAX * sizeof(float) )
#define GALLOC_SIZE_B ( SGEMM_N_MAX * SGEMM_K_MAX * sizeof(float) )
#define GALLOC_SIZE_C ( SGEMM_M_MAX * SGEMM_N_MAX * sizeof(float) )

#if defined(HAVE_DSGEMM)

#define MALLOC_SIZE_A ( GALLOC_SIZE_A * 2 )
#define MALLOC_SIZE_B ( GALLOC_SIZE_B * ( SGEMM_N_BUFFERS + 1 ) *2 )
#define MALLOC_SIZE_C ( GALLOC_SIZE_C *2 )

#else

#define MALLOC_SIZE_A ( GALLOC_SIZE_A * 1 )
#define MALLOC_SIZE_B ( GALLOC_SIZE_B * ( SGEMM_N_BUFFERS + 1 ) *1 )
#define MALLOC_SIZE_C ( GALLOC_SIZE_C *1 )

#endif

#define SGEMM_GLOBAL0_DIV 8
#define SGEMM_GLOBAL1_DIV 8

#define SGEMM_LOCAL0 8
#define SGEMM_LOCAL1 8

#define SGEMM_PAD_M 64
#define SGEMM_PAD_N 64
#define SGEMM_PAD_K 4

#define DGEMM_GLOBAL0_DIV 4
#define DGEMM_GLOBAL1_DIV 4

#define DGEMM_LOCAL0 8
#define DGEMM_LOCAL1 8

#define DGEMM_PAD_M 32
#define DGEMM_PAD_N 32
#define DGEMM_PAD_K 4 

#define CGEMM_GLOBAL0_DIV 4
#define CGEMM_GLOBAL1_DIV 4

#define CGEMM_LOCAL0 8
#define CGEMM_LOCAL1 8

#define CGEMM_PAD_M 32
#define CGEMM_PAD_N 32
#define CGEMM_PAD_K 4 

#define ZGEMM_GLOBAL0_DIV 2
#define ZGEMM_GLOBAL1_DIV 2

#define ZGEMM_LOCAL0 8
#define ZGEMM_LOCAL1 8

#define ZGEMM_PAD_M 16
#define ZGEMM_PAD_N 16
#define ZGEMM_PAD_K 4 


static char  *DEFAULT_KERNEL = "spectre";

static char  *DEFAULT_DEVICE = "spectre";

#include "../include/gpu_common.h"


