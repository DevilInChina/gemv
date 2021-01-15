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
void init_Balance_Balance2(spmv_Handle_t this_handle){
    this_handle->csrSplitter = NULL;
    this_handle->Yid = NULL;
    this_handle->Apinter = NULL;
    this_handle->Start1 = NULL;
    this_handle->End1 = NULL;
    this_handle->Start2 = NULL;
    this_handle->End2 = NULL;
    this_handle->Bpinter = NULL;
}

void init_sell_C_Sigma(spmv_Handle_t this_handle){
    this_handle->Sigma = 0;
    this_handle->C = 0;
    this_handle->banner = 0;

    this_handle->sigmaBlock = NULL;
}

/**
 * @brief free parameters used in balanced and balanced2
 * @param this_handle
 */
void clear_Balance_Balance2(spmv_Handle_t this_handle){
    free(this_handle->csrSplitter);
    free(this_handle->Yid);
    free(this_handle->Apinter);
    free(this_handle->Start1);
    free(this_handle->End1);
    free(this_handle->Start2);
    free(this_handle->End2);
    free(this_handle->Bpinter);
}

void C_Block_destory(Sigma_Block_t this_block){
    free(this_block->RowIndex);
    free(this_block->ColIndex);
    free(this_block->ValT);
    free(this_block->Y);
    free(this_block->ld);
}

void clear_Sell_C_Sigma(spmv_Handle_t this_handle) {
    if(this_handle->Sigma) {
        int siz = this_handle->banner / this_handle->Sigma;
        for (int i = 0; i < siz; ++i) {
            C_Block_destory(this_handle->sigmaBlock + i);
        }
    }
    free(this_handle->sigmaBlock);
}

void gemv_Handle_init(spmv_Handle_t this_handle){
    this_handle->spmvMethod = Method_Serial;
    this_handle->nthreads = 0;
    this_handle->cppHandle = NULL;

    init_sell_C_Sigma(this_handle);
    init_Balance_Balance2(this_handle);

}

void gemv_Handle_clear(spmv_Handle_t this_handle) {
    clear_Balance_Balance2(this_handle);
    clear_Sell_C_Sigma(this_handle);
    csr5HandleDestory(this_handle);
    gemv_Handle_init(this_handle);
}

void spmv_destory_handle(spmv_Handle_t this_handle){
    gemv_Handle_clear(this_handle);
    free(this_handle);
}

spmv_Handle_t gemv_create_handle(){
    spmv_Handle_t ret = malloc(sizeof(spmv_Handle));
    gemv_Handle_init(ret);
    return ret;
}

void spmv_clear_handle(spmv_Handle_t this_handle){
    gemv_Handle_clear(this_handle);
}






void handle_init_common_parameters(spmv_Handle_t this_handle,
                                   BASIC_SIZE_TYPE nthreads,
                                   SPMV_METHODS function,
                                   BASIC_SIZE_TYPE size,
                                   VECTORIZED_WAY vectorizedWay){
    this_handle->nthreads = nthreads;
    this_handle->vectorizedWay = vectorizedWay;
    this_handle->data_size = size;
    this_handle->spmvMethod = function;
}

const spmv_function spmv_functions[] = {
        spmv_serial_Selected,
        spmv_parallel_Selected,
        spmv_parallel_balanced_Selected,
        spmv_parallel_balanced2_Selected,
        spmv_sell_C_Sigma_Selected,
        spmv_csr5Spmv_Selected
};

void spmv_create_handle_all_in_one(spmv_Handle_t *Handle,
                                   BASIC_INT_TYPE m,
                                   BASIC_INT_TYPE n,
                                   const BASIC_INT_TYPE*RowPtr,
                                   const BASIC_INT_TYPE *ColIdx,
                                   const void *Matrix_Val,
                                   BASIC_SIZE_TYPE nthreads,
                                   SPMV_METHODS Function,
                                   BASIC_SIZE_TYPE size,
                                   VECTORIZED_WAY vectorizedWay
){
    *Handle = gemv_create_handle();
    if(Function < Method_Serial || Function >= Method_Total_Size)Function = Method_Serial;

    handle_init_common_parameters(*Handle,nthreads,Function,size,vectorizedWay);
    const int Sigma = 7744;
    const int C = 4;
    const int Turn = 8;
    const int Times = m/nthreads/C;
    switch (Function) {
        case Method_Balanced:{
            parallel_balanced_get_handle(*Handle,m,RowPtr,RowPtr[m]-RowPtr[0]);
        }break;
        case Method_Balanced2:{
            parallel_balanced2_get_handle(*Handle,m,RowPtr,RowPtr[m]-RowPtr[0]);
        }break;
        case Method_SellCSigma:{
            sell_C_Sigma_get_handle_Selected(*Handle,Times,C,m,RowPtr,ColIdx,Matrix_Val);
        }break;
        case Method_CSR5SPMV: {
            if (size == sizeof(double)) {
                csr5Spmv_get_handle_Selected(*Handle, m, n, (int *) RowPtr, (int *) ColIdx, Matrix_Val);
            }else{
                (*Handle)->spmvMethod = Method_SellCSigma;
                sell_C_Sigma_get_handle_Selected(*Handle,m/nthreads/Turn/C
                                                 ,C,m,RowPtr,ColIdx,Matrix_Val);
            }
        }break;
        default:{

            return;
        }
    }

}

void spmv(const spmv_Handle_t handle,
          BASIC_INT_TYPE m,
          const BASIC_INT_TYPE* RowPtr,
          const BASIC_INT_TYPE* ColIdx,
          const void* Matrix_Val,
          const void* Vector_Val_X,
          void*       Vector_Val_Y){
    if(handle==NULL)return;
    spmv_functions[handle->spmvMethod](handle, m, RowPtr, ColIdx, Matrix_Val, Vector_Val_X, Vector_Val_Y);
}
#define SINGLE(arg) #arg
#define STR(args1,args2) #args1 #args2
#define VEC_STRING(NAME)\
STR(NAME,_VECTOR_NONE),\
STR(NAME,_VECTOR_AVX2),\
STR(NAME,_VECTOR_AVX512)

#define ALL_FUNC_SRTING \
VEC_STRING(Method_Serial),\
VEC_STRING(Method_Parallel),\
VEC_STRING(Method_Balanced),\
VEC_STRING(Method_Balanced2),\
VEC_STRING(Method_SellCSigma),\
VEC_STRING(Method_Csr5Spmv)

const char * funcNames[]= {
    ALL_FUNC_SRTING
};
const char*Methods_names[]={
        SINGLE(Method_Serial),
        SINGLE(Method_Parallel),
        SINGLE(Method_Balanced),
        SINGLE(Method_Balanced2),
        SINGLE(Method_SellCSigma),
        SINGLE(Method_Csr5Spmv)
};
const char*Vectorized_names[]={
        SINGLE(VECTOR_NONE),
        SINGLE(VECTOR_AVX2),
        SINGLE(VECTOR_AVX512),
};