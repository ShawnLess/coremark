//====================================================================
// rocket-manycore.c
// 09/08/2018, shawnless.xie@gmail.com
//====================================================================
// routines offloding computation tasks to manycore.
//

//////////////////////////////////////////////////////////////////////
#include "rocket-manycore.h"
manycore_results           mcresults[bsg_tiles_X*bsg_tiles_Y]; // Is this outside the 22-bit address space? 

ee_u32 mcrocket_unload(core_results *crp, manycore_results *mcrp){
	crp->seed1 = mcrp->seed1;
	crp->seed2 = mcrp->seed2;
	crp->seed3 = mcrp->seed3;
	crp->size = mcrp->size;
	crp->crc = mcrp->crc;
	crp->crclist = mcrp->crclist;
	crp->crcmatrix = mcrp->crcmatrix;
	crp->crcstate = mcrp->crcstate;
	return 0;
}

int coremark_rocc_poll(void  *vp, int wait_limit){
	int volatile *pDone = (int volatile *)(vp);
	int i = 0;
	do {
		asm volatile("fence ");
		i++;
	} while((i< wait_limit) & (*pDone == 0x0));

	return i;
}

