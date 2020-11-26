
#include "mmio_highlevel.h"
#include <gemv.h>
// sum up 8 single-precision numbers


int main(int argc, char ** argv)
{
    //freopen("out.txt","w",stdout); //输出重定向，输出数据将保存在out.txt文件中
    char *filename = argv[1];
    printf ("filename = %s\n", filename);

    //read matrix
    int m, n, nnzR, isSymmetric;

    mmio_info(&m, &n, &nnzR, &isSymmetric, filename);
    int *RowPtr = (int *)malloc((m+1) * sizeof(int));
    int *ColIdx = (int *)malloc(nnzR * sizeof(int));
    float *Val    = (float *)malloc(nnzR * sizeof(float));
    mmio_data(RowPtr, ColIdx, Val, filename);
    for (int i = 0; i < nnzR; i++)
        Val[i] = 1;
    printf("The order of the rating matrix R is %i by %i, #nonzeros = %i\n",m, n, nnzR);

    //create X, Y,Y_golden
    float *X = (float *)malloc(sizeof(float) * (n+1));
    float *Y = (float *)malloc(sizeof(float) * (m+1));
    float *Y_golden = (float *)malloc(sizeof(float) * (m+1));

    memset (X, 0, sizeof(float) * (n+1));
    memset (Y, 0, sizeof(float) * (m+1));
    memset (Y_golden, 0, sizeof(float) * (m+1));

    for (int i = 0; i < n; i++)
        X[i] = 1;

    for (int i = 0; i < m; i++)
        for(int j = RowPtr[i]; j < RowPtr[i+1]; j++)
            Y_golden[i] += Val[j] * X[ColIdx[j]];

    int nthreads = atoi(argv[2]);
    omp_set_num_threads(nthreads);
    printf("#threads is %i \n", nthreads);

    int iter = atoi(argv[3]);
    //printf("#iter is %i \n", iter);

//------------------------------------serial--------------------------------
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
    int currentiter = 0;
    for (currentiter = 0; currentiter < iter; currentiter++)
    {
        serial_gemv(m,RowPtr,ColIdx,Val,X,Y);
    }
    gettimeofday(&t2, NULL);
    float time_overall_serial = ((t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0) / atoi(argv[3]);
    float GFlops_serial = 2 * nnzR / time_overall_serial / pow(10,6);
    int errorcount_serial = 0;
    for (int i = 0; i < m; i++)
        if (Y[i] != Y_golden[i])
            errorcount_serial++;

    //printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=serial-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
    //printf("time_overall_serial = %f\n", time_overall_serial);
    printf("errorcount_serial = %i\n", errorcount_serial);
    printf("GFlops_serial = %f\n", GFlops_serial);
    //printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\n");
    //free(Y);//加一
//------------------------------------------------------------------------

//-----------------------------------parallel_omp-------------------------------------
    gettimeofday(&t1, NULL);
    for (currentiter = 0; currentiter < iter; currentiter++)
    {
        parallel_gemv(m,RowPtr,ColIdx,Val,X,Y);
    }
    gettimeofday(&t2, NULL);
    float time_overall_parallel = ((t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0) / atoi(argv[3]);
    float GFlops_parallel = 2 * nnzR / time_overall_parallel / pow(10,6);
    int errorcount_parallel = 0;
    for (int i = 0; i < m; i++)
        if (Y[i] != Y_golden[i])
            errorcount_parallel++;

    //printf("-=-=-=-=-=-=-=-=-=-=-=-=-parallel_omp-=-=-=--=-=-=-=-=-=-=-=-\n");
    //printf("time_overall_parallel = %f\n", time_overall_parallel);
    printf("errorcount_parallel = %i\n", errorcount_parallel);
    printf("GFlops_parallel = %f\n", GFlops_parallel);
    //printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\n");
    //free(Y);//加一
//------------------------------------------------------------------------

//-----------------------------------parallel_omp_balanced-------------------------------------

    gemv_Handle_t balanced_handle;
    parallel_balanced_get_handle(&balanced_handle,m,RowPtr,nnzR,nthreads);
    gettimeofday(&t1, NULL);
    for (currentiter = 0; currentiter < iter; currentiter++)
    {
        parallel_balanced_gemv(balanced_handle,m,RowPtr,ColIdx,Val,X,Y);
    }
    gettimeofday(&t2, NULL);

    float time_overall_parallel_balanced = ((t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0) / atoi(argv[3]);
    float GFlops_parallel_balanced = 2 * nnzR / time_overall_parallel_balanced / pow(10,6);
    int errorcount_parallel_balanced = 0;
    for (int i = 0; i < m; i++)
        if (Y[i] != Y_golden[i])
            errorcount_parallel_balanced++;

    //printf("omp_balanced length = %d\n", length);
    //printf("-=-=-=-=-=-=-=-=-=-=parallel_omp_balanced-=-=-=-=-=-=-=-=-=-=\n");
    //printf("time_overall_parallel_balanced = %f\n", time_overall_parallel_balanced);
    printf("errorcount_parallel_balanced = %i\n", errorcount_parallel_balanced);
    printf("GFlops_parallel_balanced = %f\n", GFlops_parallel_balanced);
    //printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\n");
    //free(Y);//加一
//------------------------------------------------------------------------

//-----------------------------------parallel_omp_balanced_Yid-------------------------------------

    gettimeofday(&t1, NULL);
    gemv_Handle_t balanced2_handle;
    parallel_balanced_get_handle(&balanced2_handle,m,RowPtr,nnzR,nthreads);
    for (currentiter = 0; currentiter < iter; currentiter++)
    {
        parallel_balanced2_gemv(
                                balanced2_handle,
                                m,RowPtr,ColIdx,Val,X,Y
        );
    }
    gettimeofday(&t2, NULL);
    float time_overall_parallel_omp_balanced_Yid = ((t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0) / atoi(argv[3]);
    float GFlops_parallel_omp_balanced_Yid = 2 * nnzR / time_overall_parallel_omp_balanced_Yid / pow(10,6);
    int errorcount_parallel_omp_balanced_Yid = 0;
    for (int i = 0; i < m; i++)
        if (Y[i] != Y_golden[i])
            errorcount_parallel_omp_balanced_Yid++;

    //printf("-=-=-=-=-=-=-=-=-=-=-=-=-parallel_omp-=-=-=--=-=-=-=-=-=-=-=-\n");
    //printf("time_overall_parallel = %f\n", time_overall_parallel);
    printf("errorcount_parallel_omp_balanced_Yid = %i\n", errorcount_parallel_omp_balanced_Yid);
    printf("GFlops_parallel_omp_balanced_Yid = %f\n", GFlops_parallel_omp_balanced_Yid);
    //printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\n");
    //free(Y);//加一
//-----------------------------------------------------------------------

//------------------------------------parallel_omp_balanced_avx2------------------------------------
    gettimeofday(&t1, NULL);
    for (currentiter = 0; currentiter < iter; currentiter++)
    {
        parallel_balanced_gemv_avx2(balanced_handle,m,RowPtr,ColIdx,Val,X,Y);
    }
    gettimeofday(&t2, NULL);
    float time_overall_parallel_avx2 = ((t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0) / atoi(argv[3]);
    float GFlops_parallel_avx2 = 2 * nnzR / time_overall_parallel_avx2 / pow(10,6);
    int errorcount_parallel_avx2 = 0;
    for (int i = 0; i < m; i++)
        if (Y[i] != Y_golden[i])
            errorcount_parallel_avx2++;

    //printf("-=-=-=-=-=-=-=-=parallel_omp_balanced_avx2-=-=-=-=-=-=-=-=-=-\n");
    //printf("time_overall_parallel_avx2 = %f\n", time_overall_parallel_avx2);
    printf("errorcount_parallel_avx2 = %i\n", errorcount_parallel_avx2);
    printf("GFlops_parallel_avx2 = %f\n", GFlops_parallel_avx2);
    //printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\n");
    //free(Y);//加一
//------------------------------------------------------------------------

//------------------------------------parallel_omp_balanced_avx512------------------------------------
    gettimeofday(&t1, NULL);
    for (currentiter = 0; currentiter < iter; currentiter++)
    {
        parallel_balanced_gemv_avx512(balanced_handle,m,RowPtr,ColIdx,Val,X,Y);
    }
    gettimeofday(&t2, NULL);
    float time_overall_parallel_avx512 = ((t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0) / atoi(argv[3]);
    float GFlops_parallel_avx512 = 2 * nnzR / time_overall_parallel_avx512 / pow(10,6);
    int errorcount_parallel_avx512 = 0;
    for (int i = 0; i < m; i++)
        if (Y[i] != Y_golden[i])
            errorcount_parallel_avx512++;

    //printf("-=-=-=-=-=-=-=-=parallel_omp_balanced_avx512-=-=-=-=-=-=-=-=-\n");
    //printf("time_overall_parallel_avx512 = %f\n", time_overall_parallel_avx512);
    printf("errorcount_parallel_avx512 = %i\n", errorcount_parallel_avx512);
    printf("GFlops_parallel_avx512 = %f\n", GFlops_parallel_avx512);
    //printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\n");
    //free(Y);//加一
//------------------------------------------------------------------------

//------------------------------------parallel_omp_balanced_avx2_Yid------------------------------------
    gettimeofday(&t1, NULL);
    for (currentiter = 0; currentiter < iter; currentiter++) {
        parallel_balanced2_gemv_avx2(balanced2_handle,m,RowPtr,ColIdx,Val,X,Y);
    }
    gettimeofday(&t2, NULL);
    float time_overall_parallel_avx2_Yid = ((t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0) / atoi(argv[3]);
    float GFlops_parallel_avx2_Yid = 2 * nnzR / time_overall_parallel_avx2_Yid / pow(10,6);
    int errorcount_parallel_avx2_Yid = 0;
    for (int i = 0; i < m; i++)
        if (Y[i] != Y_golden[i])
            errorcount_parallel_avx2_Yid++;

    //printf("-=-=-=-=-=-=-=-=parallel_omp_balanced_avx2-=-=-=-=-=-=-=-=-=-\n");
    //printf("time_overall_parallel_avx2 = %f\n", time_overall_parallel_avx2);
    printf("errorcount_parallel_avx2_Yid = %i\n", errorcount_parallel_avx2_Yid);
    printf("GFlops_parallel_avx2_Yid = %f\n", GFlops_parallel_avx2_Yid);
    //printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\n");
    //free(Y);//加一
//------------------------------------------------------------------------

//------------------------------------parallel_omp_balanced_avx512_Yid-----------------------------------
    gettimeofday(&t1, NULL);
    for (currentiter = 0; currentiter < iter; currentiter++)
    {
        parallel_balanced2_gemv_avx512(balanced2_handle,m,RowPtr,ColIdx,Val,X,Y);
    }
    gettimeofday(&t2, NULL);
    float time_overall_parallel_avx512_Yid = ((t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0) / atoi(argv[3]);
    float GFlops_parallel_avx512_Yid = 2 * nnzR / time_overall_parallel_avx512_Yid / pow(10,6);
    int errorcount_parallel_avx512_Yid = 0;
    for (int i = 0; i < m; i++)
        if (Y[i] != Y_golden[i])
            errorcount_parallel_avx512_Yid++;

    //printf("-=-=-=-=-=-=-=-=parallel_omp_balanced_avx2-=-=-=-=-=-=-=-=-=-\n");
    //printf("time_overall_parallel_avx2 = %f\n", time_overall_parallel_avx2);
    printf("errorcount_parallel_avx512_Yid = %i\n", errorcount_parallel_avx512_Yid);
    printf("GFlops_parallel_avx512_Yid = %f\n", GFlops_parallel_avx512_Yid);
    //printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\n");
    //free(Y);//加一
//------------------------------------------------------------------------

    free(X);//加一
    free(Y);//加一
    free(Y_golden);//加一
    gemv_destory_handle(balanced_handle);
    gemv_destory_handle(balanced2_handle);
}