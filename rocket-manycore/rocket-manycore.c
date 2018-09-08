//====================================================================
// rocket-manycore.c
// 09/08/2018, shawnless.xie@gmail.com
//====================================================================
// routines offloding computation tasks to manycore.
//

//////////////////////////////////////////////////////////////////////
#include "rocket-manycore.h"

// Global variables decleared in the manycore program / memory space. 
// pMC_MatrixData is the start address for matrix A in manycore
// B = MANYCORE_ALIGN_MEM(A + N*N)
// C = (MATRES *)MANYCORE_MEM( B + N*N)
extern MATDAT              *pMC_MatrixData;               
// pMC_manycore_mat_params is the matrix parameters in the manycore memory space
extern manycore_mat_params *pMC_manycore_mat_params;
extern int                 *manycore_mem_vec;

// gRC_manycore_mat_params holds the matrix offload parameters
manycore_mat_params         gRC_manycore_mat_params;

ee_u32 rocket_init_matrix(ee_u32 blksize, void *memblk, ee_s32 seed, mat_params *p){
	ee_u32 ret;
	ret = core_init_matrix(blksize, memblk, seed, p);
	gRC_manycore_mat_params.A = p->A;
	gRC_manycore_mat_params.B = p->B;
	gRC_manycore_mat_params.C = p->C;
	gRC_manycore_mat_params.N = p->N;
	gRC_manycore_mat_params.base_addr = (ee_u32) &pMC_manycore_mat_params;
	return manycore_init_matrix(&gRC_manycore_mat_params);
}

ee_u32 rocket_bench_matrix(mat_params *p, ee_s16 seed, ee_u16 crc){
        ee_u32 rcrc;
	// I would consider this dangerous, but I don't see any
	// alternative. Passing a global variable like gRC_manycore_mat_params
	// is just bad form, but I don't see any other way. 
	manycore_bench_matrix_nb(&gRC_manycore_mat_params, 0, 0, seed, crc);
	coremark_rocc_poll((void*)&(gRC_manycore_mat_params.done), 100000);
	bsg_rocc_read( 0, 0, &(pMC_manycore_mat_params->crc), rcrc );
	return rcrc;
}

/* Initialize the matrix benchmark parameters on the manycore processor */
ee_u32 manycore_init_matrix(manycore_mat_params* mcp) {
#if  (MATDAT_INT != 1)
#error ("Only supports matrix datatype 'int'")
#endif

	manycore_mat_params *pRC_manycore_mat_params;
	unsigned int i;
	ee_u32 N = mcp->N;

	// Compute the matrix pointers in the manycore memory space
	MATDAT *pMC_A = pMC_MatrixData; // Global Variable
	MATDAT *pMC_B = (MATDAT*)MANYCORE_ALIGN_MEM(pMC_A + N*N); 
	MATRES *pMC_C = (MATRES*)MANYCORE_ALIGN_MEM(pMC_B + N*N);

	// Get Rocket Memory Space pointers to the manycore_mat_params struct,
	// and matricies in the Manycore memory space
	pRC_manycore_mat_params = MC2RC_PTR(pMC_manycore_mat_params, 
					*manycore_mem_vec); // Global Variable
	MATDAT *pRC_A = MC2RC_PTR(pMC_A, *manycore_mem_vec); // Global Variable
	MATDAT *pRC_B = MC2RC_PTR(pMC_B, *manycore_mem_vec); // Global Variable
	MATDAT *pRC_C = MC2RC_PTR(pMC_C, *manycore_mem_vec); // Global Variable

	// Copy the matricies A & B
	for(i = 0; i < N*N; ++i){
		pRC_A[i] = mcp->A[i];
		pRC_B[i] = mcp->B[i];
		pRC_C[i] = 0;
	}

	// Set the parameters in the manycore memory space
	pRC_manycore_mat_params->N = N;
	pRC_manycore_mat_params->A = pMC_A; 
	pRC_manycore_mat_params->B = pMC_B;
	pRC_manycore_mat_params->C = pMC_C;
	pRC_manycore_mat_params->base_addr = mcp->base_addr;

	// Load the manycore program at processor x = 0, y= 0
	// 
#ifndef DMA_LOAD
	bsg_rocc_load_manycore(0, 0);
#else
	bsg_rocc_dma_load_manycore_nb(0, 0);
	bsg_rocket_fence( );
#endif

#if CORE_DEBUG
	printmat(pMC_A,N,"A (Manycore)");
	printmat(pMC_B,N,"B (Manycore)");
#endif
	return N;
}

// The mat_params *p is replaced with y_cord and x_cord.
// pMC_manycore_mat_params is the matrix parameters in manycore
// This is a non-blocking call, so we do not wait for the result
void manycore_bench_matrix_nb(manycore_mat_params *p, 
			ee_u32 y, ee_u32 x, ee_s16 seed, ee_u16 crc){
	ee_u32 seed32 = seed;
	ee_u32 crc32 = crc

        bsg_rocc_write( y, x, &(pMC_manycore_mat_params->seed), seed32 );

        bsg_rocc_write( y, x, &(pMC_manycore_mat_params->crc), crc32 );
	
	// Clear the done flag
        p->done = 0;

        //start the tile
        bsg_rocc_unfreeze(y, x);
}

int cmark_rocc_poll(void  *pRCViewDone, int wait_limit) {
	int volatile *pDone = (int volatile *)(pRCViewDone);
	int i = 0;
	do {
		asm volatile("fence ");
		i++;
	} while((i< wait_limit) & (*pDone == 0x0));

	return i;
}

