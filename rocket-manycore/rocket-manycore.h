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
#include <stdio.h>
#include <stdlib.h>

#ifndef MANYCORE_PROG
   #include "bsg_rocket_rocc.h"
#endif

#include "bsg_manycore_buffer.h"
#include "manycore.cfg.h"
#include "coremark.h"
//The APIs for calling RoCC interface.
typedef struct manycore_results_ {
        // base_addr is the base address that this struct has been mapped to in
        // the Rocket memory space. When manycore stores data back to the
        // rocket, it should add the base_addr to the buffer to get the right
        // address
        //ee_u32 addr; // TODO: Double check this is in tile memory space!!!
        // Return value
        ee_u32 result;
	// Done Flag
        ee_u32 done;

	// Write these when iterations are complete!!!
	ee_s16 seed1, seed2, seed3;
	ee_u32 size;
	ee_u16 crc, crclist, crcmatrix, crcstate;
} manycore_results;

#ifndef MANYCORE_PROG
// Defined in manycores code
extern core_results        *bresults;
extern manycore_results    *_mcresult;
extern unsigned long       *mcmemblk;

// Defined in rocket-manycore.c
extern manycore_results           mcresults[bsg_tiles_X*bsg_tiles_Y]; // Is this outside the 22-bit address space? 
#endif

//////////////////////////////////////////////////////////////////////
/* matrix benchmark functions */

ee_u32 mcrocket_init(manycore_results *manycore_p, manycore_results *rocket_p);
ee_u32 mcrocket_unload(core_results *crp, manycore_results *mcrp);
int coremark_rocc_poll(void *vp, int wait_limit);

#define MANYCORE_ALIGN_MEM(x) (void *)(4 + (((ee_u32)(x) - 1) & ~3))
#define ROCKET_ALIGN_MEM(x) (void *)(4 + (((unsigned long long )(x) - 1) & ~3))
#define MC2RC_PTR(x, p) (void *)((unsigned long long)(x) + (p))
#endif
