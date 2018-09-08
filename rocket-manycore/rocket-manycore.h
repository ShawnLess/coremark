//====================================================================
// rocket-manycore.h
// 09/08/2018, shawnless.xie@gmail.com
//====================================================================
// Header files for offloding computation tasks to manycore.
//
#ifndef _ROCKET_MANYCORE_H_
#define _ROCKET_MANYCORE_H_

//////////////////////////////////////////////////////////////////////
#include "coremark.h"
//The APIs for calling RoCC interface.
#include "bsg_rocket_rocc.h"


//////////////////////////////////////////////////////////////////////
/* matrix benchmark functions */
ee_u32 manycore_init_matrix(ee_u32 blksize, void *memblk, ee_s32 seed, mat_params *p);

ee_u16 manycore_bench_matrix(mat_params *p, ee_s16 seed, ee_u16 crc);

//This is the placeholder for matrix bench parameters.
typedef struct matrix_bench_param_ {
        mat_param       s_mat_params;
        //we only supports 32 bits transfer, so we need to cast the data type locally.
        ee_u32          seed;
        ee_u32          crc;

        //the base address that this struct has been mapped.
        //When manycore try to store data back to the rocket, it 
        //should added the base_addr to the buffer to get the right 
        //address
        ee_u32          base_addr;
        //the return value and done signals
        ee_u32          result;
        ee_u32          done;
} matrix_bench_param;

int  bsg_rocc_poll_task(void  *pRCViewDone, int waiting_cycle_limit) {
    int volatile *pDone = (int volatile * )( pRCViewDone );
    int i=0;
    do {
        asm volatile( "fence ");
        i++;
      }while( (i< waiting_cycle_limit ) & ( *pDone == 0x0) );

    return i;
}

#define manycore_align_mem(x) (void *)(4 + (((ee_u32)(x) - 1) & ~3))
#define rocket_align_mem(x)   (void *)(4 + (((unsgined long long )(x) - 1) & ~3))

#define MC2RC_VIEW_PTR(x, offset) (void *)((unsigned long long)(x) + (offset))

#endif
