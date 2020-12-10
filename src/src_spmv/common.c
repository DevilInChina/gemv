//
// Created by kouushou on 2020/11/25.
//

#include "inner_spmv.h"
#include <math.h>
#include <string.h>
/**
 * @brief init parameters used in balanced and balanced2
 * @param this_handle
 */
void init_Balance_Balance2(gemv_Handle_t this_handle){
    this_handle->csrSplitter = NULL;
    this_handle->Yid = NULL;
    this_handle->Apinter = NULL;
    this_handle->Start1 = NULL;
    this_handle->End1 = NULL;
    this_handle->Start2 = NULL;
    this_handle->End2 = NULL;
    this_handle->Bpinter = NULL;
}

void init_sell_C_Sigma(gemv_Handle_t this_handle){
    this_handle->Sigma = 0;
    this_handle->C = 0;
    this_handle->banner = 0;

    this_handle->C_Blocks = NULL;
}

/**
 * @brief free parameters used in balanced and balanced2
 * @param this_handle
 */
void clear_Balance_Balance2(gemv_Handle_t this_handle){
    free(this_handle->csrSplitter);
    free(this_handle->Yid);
    free(this_handle->Apinter);
    free(this_handle->Start1);
    free(this_handle->End1);
    free(this_handle->Start2);
    free(this_handle->End2);
    free(this_handle->Bpinter);
}

void C_Block_destory(C_Block_t this_block){
    free(this_block->RowIndex);
    free(this_block->ColIndex);
    free(this_block->ValT);
    free(this_block->Y);
}

void clear_Sell_C_Sigma(gemv_Handle_t this_handle) {
    int siz = this_handle->banner / (this_handle->C ? this_handle->C : 1);
    for (int i = 0; i < siz; ++i) {
        C_Block_destory(this_handle->C_Blocks + i);
    }
    free(this_handle->C_Blocks);
}

void gemv_Handle_init(gemv_Handle_t this_handle){
    this_handle->status = STATUS_NONE;
    this_handle->nthreads = 0;

    init_Balance_Balance2(this_handle);
    init_sell_C_Sigma(this_handle);

}

void gemv_Handle_clear(gemv_Handle_t this_handle) {
    clear_Balance_Balance2(this_handle);
    clear_Sell_C_Sigma(this_handle);

    gemv_Handle_init(this_handle);
}

void gemv_destory_handle(gemv_Handle_t this_handle){
    gemv_Handle_clear(this_handle);
    free(this_handle);
}

gemv_Handle_t gemv_create_handle(){
    gemv_Handle_t ret = malloc(sizeof(gemv_Handle));
    gemv_Handle_init(ret);
    return ret;
}

void gemv_clear_handle(gemv_Handle_t this_handle){
    gemv_Handle_clear(this_handle);
}






void handle_init_common_parameters(gemv_Handle_t this_handle,
                                   BASIC_SIZE_TYPE nthreads,
                                   STATUS_GEMV_HANDLE function,
                                   BASIC_SIZE_TYPE size,
                                   VECTORIZED_WAY vectorizedWay){
    this_handle->nthreads = nthreads;
    this_handle->vectorizedWay = vectorizedWay;
    this_handle->data_size = size;
    this_handle->status = function;
}

const spmv_function spmv_functions[] = {
        spmv_serial_Selected,
        spmv_parallel_Selected,
        spmv_parallel_balanced_Selected,
        spmv_parallel_balanced2_Selected,
        spmv_sell_C_Sigma_Selected
};

void spmv_create_handle_all_in_one(gemv_Handle_t *Handle,
                                   BASIC_INT_TYPE m,
                                   const BASIC_INT_TYPE*RowPtr,
                                   const BASIC_INT_TYPE *ColIdx,
                                   const void *Matrix_Val,
                                   BASIC_SIZE_TYPE nthreads,
                                   STATUS_GEMV_HANDLE Function,
                                   BASIC_SIZE_TYPE size,
                                   VECTORIZED_WAY vectorizedWay
){
    *Handle = gemv_create_handle();
    if(Function<STATUS_NONE || Function>=STATUS_TOTAL_SIZE)Function = STATUS_NONE;

    handle_init_common_parameters(*Handle,nthreads,Function,size,vectorizedWay);

    switch (Function) {
        case STATUS_BALANCED:{
            parallel_balanced_get_handle(*Handle,m,RowPtr,RowPtr[m]-RowPtr[0]);
        }break;
        case STATUS_BALANCED2:{
            parallel_balanced2_get_handle(*Handle,m,RowPtr,RowPtr[m]-RowPtr[0]);
        }break;
        case STATUS_SELL_C_SIGMA:{
            sell_C_Sigma_get_handle_Selected(*Handle,4,32,m,RowPtr,ColIdx,Matrix_Val);
        }
        default:{

            return;
        }
    }
}

void spmv(const gemv_Handle_t handle,
          BASIC_INT_TYPE m,
          const BASIC_INT_TYPE* RowPtr,
          const BASIC_INT_TYPE* ColIdx,
          const void* Matrix_Val,
          const void* Vector_Val_X,
          void*       Vector_Val_Y){
    if(handle==NULL)return;
    spmv_functions[handle->status](handle,m,RowPtr,ColIdx,Matrix_Val,Vector_Val_X,Vector_Val_Y);
}

#define STR(args1,args2) #args1 #args2

#define VEC_STRING(NAME)\
STR(NAME,_VECTOR_NONE),\
STR(NAME,_VECTOR_AVX2),\
STR(NAME,_VECTOR_AVX512)

#define ALL_FUNC_SRTING \
VEC_STRING(STATUS_NONE),\
VEC_STRING(STATUS_PARALLEL),\
VEC_STRING(STATUS_BALANCED),\
VEC_STRING(STATUS_BALANCED2),\
VEC_STRING(STATUS_SELL_C_SIGMA)

const char * funcNames[]= {
    ALL_FUNC_SRTING
};