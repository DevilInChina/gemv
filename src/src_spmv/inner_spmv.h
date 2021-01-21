//
// Created by kouushou on 2020/12/10.
//

#ifndef GEMV_INNER_SPMV_H
#define GEMV_INNER_SPMV_H
#include <spmv.h>

/**
 * @brief create a empty handle with initialize
 * @return
 */
spmv_Handle_t gemv_create_handle();
/**
 *
 * @param handle
 * @param m
 * @param RowPtr
 * @param nnzR
 * @param nthreads
 */
void parallel_balanced_get_handle(
        spmv_Handle_t handle,
        BASIC_INT_TYPE m,
        const BASIC_INT_TYPE*RowPtr,
        BASIC_INT_TYPE nnzR);

/**
 *
 * @param handle
 * @param m
 * @param RowPtr
 * @param nnzR
 * @param nthreads
 */
void parallel_balanced2_get_handle(
        spmv_Handle_t handle,
        BASIC_INT_TYPE m,
        const BASIC_INT_TYPE*RowPtr,
        BASIC_INT_TYPE nnzR);

void sell_C_Sigma_get_handle_Selected(spmv_Handle_t handle,
                                      BASIC_INT_TYPE Times, BASIC_INT_TYPE C,
                                      BASIC_INT_TYPE m,
                                      const BASIC_INT_TYPE*RowPtr,
                                      const BASIC_INT_TYPE*ColIdx,
                                      const void*Matrix_Val
) ;

int binary_search_right_boundary_kernel(const int *row_pointer,
                                        const int  key_input,
                                        const int  size);

void handle_init_common_parameters(spmv_Handle_t this_handle,
                                   BASIC_SIZE_TYPE nthreads,
                                   SPMV_METHODS function,
                                   BASIC_SIZE_TYPE size,
                                   VECTORIZED_WAY vectorizedWay);

void spmv_parallel_balanced_Selected(
        const spmv_Handle_t handle,
        BASIC_INT_TYPE m,
        const BASIC_INT_TYPE* RowPtr,
        const BASIC_INT_TYPE* ColIdx,
        const void* Matrix_Val,
        const void* Vector_Val_X,
        void*       Vector_Val_Y);

void spmv_parallel_balanced2_Selected(
        const spmv_Handle_t handle,
        BASIC_INT_TYPE m,
        const BASIC_INT_TYPE* RowPtr,
        const BASIC_INT_TYPE* ColIdx,
        const void* Matrix_Val,
        const void* Vector_Val_X,
        void*       Vector_Val_Y
);


void spmv_sell_C_Sigma_Selected(const spmv_Handle_t handle,
                                BASIC_INT_TYPE m,
                                const BASIC_INT_TYPE* RowPtr,
                                const BASIC_INT_TYPE* ColIdx,
                                const void* Matrix_Val,
                                const void* Vector_Val_X,
                                void*       Vector_Val_Y
);

void spmv_serial_Selected(const spmv_Handle_t handle,
                          BASIC_INT_TYPE m,
                          const BASIC_INT_TYPE* RowPtr,
                          const BASIC_INT_TYPE* ColIdx,
                          const void* Matrix_Val,
                          const void* Vector_Val_X,
                          void*       Vector_Val_Y);

void spmv_parallel_Selected(const spmv_Handle_t handle,
                            BASIC_INT_TYPE m,
                            const BASIC_INT_TYPE* RowPtr,
                            const BASIC_INT_TYPE* ColIdx,
                            const void* Matrix_Val,
                            const void* Vector_Val_X,
                            void*       Vector_Val_Y
);

int numa_spmv_get_handle_Selected(spmv_Handle_t handle,
                                  BASIC_INT_TYPE m,BASIC_INT_TYPE n,
                                  const BASIC_INT_TYPE *RowPtr,
                                  const BASIC_INT_TYPE *ColIdx,
                                  const void *Matrix_Val
);
#if defined(__cplusplus)
extern "C" {
#endif
void csr5HandleDestory(spmv_Handle_t handle);

void numaHandleDestory(spmv_Handle_t handle);

void csr5Spmv_get_handle_Selected(spmv_Handle_t handle,
                                  BASIC_INT_TYPE m,
                                  BASIC_INT_TYPE n,
                                  BASIC_INT_TYPE*RowPtr,
                                  BASIC_INT_TYPE*ColIdx,
                                  const void*Matrix_Val
) ;

void spmv_csr5Spmv_Selected(const spmv_Handle_t handle,
                            BASIC_INT_TYPE m,
                            const BASIC_INT_TYPE* RowPtr,
                            const BASIC_INT_TYPE* ColIdx,
                            const void* Matrix_Val,
                            const void* Vector_Val_X,
                            void*       Vector_Val_Y
);

void spmv_numa_Selected(
        const spmv_Handle_t handle,
        BASIC_INT_TYPE m,
        const BASIC_INT_TYPE* RowPtr,
        const BASIC_INT_TYPE* ColIdx,
        const void* Matrix_Val,
        const void* Vector_Val_X,
        void*       Vector_Val_Y
);

#if defined(__cplusplus)
}
#endif



typedef void(*spmv_function) (const spmv_Handle_t handle,
                      BASIC_INT_TYPE m,
                      const BASIC_INT_TYPE* RowPtr,
                      const BASIC_INT_TYPE* ColIdx,
                      const void* Matrix_Val,
                      const void* Vector_Val_X,
                      void*       Vector_Val_Y
);



extern const spmv_function spmv_functions[];

int lower_bound(const int *first,const int *last,int key);

int upper_bound(const int *first,const int *last,int key);

#endif //GEMV_INNER_SPMV_H
