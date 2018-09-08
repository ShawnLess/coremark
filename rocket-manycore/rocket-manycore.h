//====================================================================
// rocket-manycore.h
// 09/08/2018, shawnless.xie@gmail.com
//====================================================================
// Header files for offloding computation tasks to manycore.
//
#ifndef _ROCKET_MANYCORE_H_
#define _ROCKET_MANYCORE_H_

//////////////////////////////////////////////////////////////////////
#include <inttypes.h>
#include "bsg_rocket_rocc.h"
#include "coremark.h"
//The APIs for calling RoCC interface.


//This is the placeholder for matrix bench parameters.
typedef struct manycore_mat_params_ {
	// These values are set during initialization (rocket_init_matrix)
	ee_u32 N;

        // base_addr is the base address that this struct has been mapped to in
        // the Rocket memory space. When manycore stores data back to the
        // rocket, it should add the base_addr to the buffer to get the right
        // address
        ee_u32 base_addr;
	
	// Matrix pointers
	MATDAT *A, *B;
	MATRES *C;

	// These values are set during the benchmark run
        ee_u32 seed;
        ee_u32 crc;

        // Return value
        ee_u32 result;
	// Done Flag
        ee_u32 done;
} manycore_mat_params;

//////////////////////////////////////////////////////////////////////
/* matrix benchmark functions */
ee_u32 manycore_init_matrix(manycore_mat_params* mcp);

void manycore_bench_matrix_nb(manycore_mat_params *p, 
			ee_u32 y, ee_u32 x, ee_s16 seed, ee_u16 crc);


#define MANYCORE_ALIGN_MEM(x) (void *)(4 + (((ee_u32)(x) - 1) & ~3))
#define ROCKET_ALIGN_MEM(x) (void *)(4 + (((unsigned long long )(x) - 1) & ~3))
#define MC2RC_PTR(x, p) (void *)((unsigned long long)(x) + (p))
#endif
