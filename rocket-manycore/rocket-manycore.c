//====================================================================
// rocket-manycore.c
// 09/08/2018, shawnless.xie@gmail.com
//====================================================================
// routines offloding computation tasks to manycore.
//

//////////////////////////////////////////////////////////////////////
#include "rocket-manycore.h"

//variables decleared in mancyore program
extern MATDAT *                  pMC_MatrixData;               
extern matrix_bench_param *      pMC_matrix_bench_param;
extern int                       manycore_mem_vec[]   ;

//Global variables for holind the matrix offloading parameters
matrix_bench_param               gRC_matrix_bench_param;

// pMC_MatrixData is the start address for matrix A in manycore
// B = manycore_align_mem(A + N*N)
// C = (MATRES *)manycore_mem( B + N*N)

// pMC_matrix_bench_param is the matrix parameters in manycore

/*initilized the matrix parameters*/
ee_u32 manycore_init_matrix(    ee_u32 base_addr,                                       \
                                ee_u32 *p_manycore_mem_vect,                             \
                                ee_u32 N, mat_params *p) {

  matrix_bench_param *      pRC_matrix_bench_param = gRC_matrix_bench_param ;
  //points to the structure in the local mancyore images
  matrix_bench_param *      pRCView_MC_matrix_bench_param = pMC_matrix_bench_param \
                                                          + p_manycore_mem_vect;
  unsigned int              i;
  ee_u32                    N= pRC_matrix_bench_param->N;

  //setup the base address
  pRCView_MC_matrix_bench_param->base_addr = base_addr;

  //set the parametes
  MATDAT *pA = pMC_MatrixData;                  
  MATDAT *pB = manycore_align_mem( pA + N*N); 
  MATRES *pC = manycore_align_mem( pB + N*N);

  pRCView_MC_matrix_bench_param->N  =  pRC_matrix_bench_param->N;
  pRCView_MC_matrix_bench_param->pA =  pA; 
  pRCView_MC_matrix_bench_param->pB =  pB;
  pRCView_MC_matrix_bench_param->pC =  pC;

  #if  (MATDAT_INT != 1)
        #error ("Only supports matrix int data now")
  #endif
  //copy the matrix A & B
  MATDAT *pRCView_A = MC2RC_VIEW_PTR( pA, p_manycore_mem_vect );                  
  MATDAT *pRCView_B = MC2RC_VIEW_PTR( pB, p_manycore_mem_vect );                  
  for( i=0; i<N*N; i++){
        pRCView_A[i] = pRC_matrix_bench_param->A[i] ;
        pRCView_B[i] = pRC_matrix_bench_param->B[i] ;
  }

}

// The mat_params *p is replaced with y_cord and x_cord.
// pMC_matrix_bench_param is the matrix parameters in manycore
void manycore_bench_matrix_nb(   ee_u32 y_cord, ee_u32 x_cord,                           \
                                ee_s16 seed, ee_u16 crc){
        matrix_bench_param *      pRC_matrix_bench_param = gRC_matrix_bench_param ;

        //set the seed value
        bsg_rocc_write( y_cord, x_cord, &(pMC_matrix_bench_param->seed),  ee_u32(seed));
        //set the crc  value
        bsg_rocc_write( y_cord, x_cord, &(pMC_matrix_bench_param->crc),  ee_u32(crc));
        //reset the done signal.
        pRC_matrix_bench_param->done = 0;
        //start the tile
        bsg_rocc_unfreeze( y_cord, x_cord);
        //this is a non-block call, so we do not wait the result
}

int  bsg_rocc_poll(void  *pRCViewDone, int waiting_cycle_limit=100000) {
    int volatile *pDone = (int volatile * )( pRCViewDone );
    int i=0;
    do {
        asm volatile( "fence ");
        i++;
      }while( (i< waiting_cycle_limit ) & ( *pDone == 0x0) );

    return i;
}
#endif
