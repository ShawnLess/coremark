#include "coremark.h"
#include "rocket-manycore.h"

core_results        *bresults;
manycore_results    *_mcresult;
int                 *manycore_mem_vec;
unsigned long       mcmemblk[512];

void main(){
	void *ret;
	ret = iterate(bresults);
	_mcresult->seed1 = bresults->seed1;
	_mcresult->seed2 = bresults->seed2;
	_mcresult->seed3 = bresults->seed3;
	_mcresult->size = bresults->size;

	_mcresult->crc = bresults->crc;
	_mcresult->crclist = bresults->crcresult;
	_mcresult->crcmatrix = bresults->crcmatrix;
	_mcresult->crcstate = bresults->crcstate;
	_mcresult->done = 1;
}

void *iterate(void *pres) {
	ee_u32 i;
	ee_u16 crc;
	core_results *res=(core_results *)pres;
	ee_u32 iterations=res->iterations;
	res->crc=0;
	res->crclist=0;
	res->crcmatrix=0;
	res->crcstate=0;

	for (i=0; i<iterations; i++) {
		crc=core_bench_list(res,1);
		res->crc=crcu16(crc,res->crc);
		crc=core_bench_list(res,-1);
		res->crc=crcu16(crc,res->crc);
		if (i==0) res->crclist=res->crc;
	}
	return NULL;
}
