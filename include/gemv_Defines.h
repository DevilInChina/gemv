//
// Created by kouushou on 2020/12/6.
//

#ifndef GEMV_GEMV_DEFINES_H
#define GEMV_GEMV_DEFINES_H

#ifndef GEMV_INT_TYPE
#define GEMV_INT_TYPE int
#endif

#ifndef GEMV_VAL_TYPE
#define GEMV_VAL_TYPE double
#endif


typedef enum DOT_PRODUCT_WAY{
    DOT_NONE,
    DOT_AVX2,
    DOT_AVX512
}DOT_PRODUCT_WAY;


typedef enum STATUS_GEMV_HANDLE{
    STATUS_NONE,
    STATUS_BALANCED,
    STATUS_BALANCED2,
    STATUS_SELL_C_SIGMA
}STATUS_GEMV_HANDLE;
typedef struct gemv_Handle {
    STATUS_GEMV_HANDLE status;


    ///------balanced balanced2------///
    GEMV_INT_TYPE nthreads;
    GEMV_INT_TYPE* csrSplitter;
    GEMV_INT_TYPE* Yid;
    GEMV_INT_TYPE* Apinter;
    GEMV_INT_TYPE* Start1;
    GEMV_INT_TYPE* End1;
    GEMV_INT_TYPE* Start2;
    GEMV_INT_TYPE* End2;
    GEMV_INT_TYPE* Bpinter;
    ///------balanced balanced2------///


    ///---------sell C Sigma---------///
    GEMV_INT_TYPE Sigma,C;
    GEMV_INT_TYPE banner;
    GEMV_VAL_TYPE** ValCSC;
    GEMV_INT_TYPE** COLCSC;
    GEMV_INT_TYPE** ROWCSC;
    ///---------sell C Sigma---------///
}gemv_Handle;

typedef gemv_Handle*  gemv_Handle_t;

typedef enum LINE_PRODUCT_WAY{
    GEMV_LINE_PRODUCT_4,
    GEMV_LINE_PRODUCT_4_AVX2,
    GEMV_LINE_PRODUCT_4_AVX512,
    GEMV_LINE_PRODUCT_8,
    GEMV_LINE_PRODUCT_8_AVX2,
    GEMV_LINE_PRODUCT_8_AVX512,
    GEMV_LINE_PRODUCT_16,
    GEMV_LINE_PRODUCT_16_AVX2,
    GEMV_LINE_PRODUCT_16_AVX512,
}LINE_PRODUCT_WAY;
#endif //GEMV_GEMV_DEFINES_H