#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <sys/time.h>
#include <gemv.h>
#include "mmio_highlevel.h"

void printmat(double *A, int n)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
            printf("%4.8f ", A[i * n + j]);
        printf("\n");
    }
}

void printvec(double *x, int n)
{
    for (int i = 0; i < n; i++)
        printf("%4.8f\n", x[i]);
}

double vec2norm(double *x, int n)
{
    double sum = 0;
    for (int i = 0; i < n; i++)
        sum += x[i] * x[i];
    return sqrt(sum);
}
float vec2norm_float(float *x,int n)
{
   float sum = 0;
    for (int i = 0; i < n; i++)
        sum += x[i] * x[i];
    return sqrt(sum);
}

void matvec(double *A, double *x, double *y, int n)
{
    for (int i = 0; i < n; i++)
    {
        y[i] = 0;
        for (int j = 0; j < n; j++)
            y[i] += A[i * n + j] * x[j];
    }
}


double dotproduct(double *x1, double *x2, int n)
{
    double sum = 0;
    for (int i = 0; i < n; i++)
        sum += x1[i] * x2[i];
    return sum;
}
float dotproduct_float(float *x1,float *x2,int n)
{
    float  sum=0;
    for (int i = 0; i < n; i++)
        sum += x1[i] * x2[i];
    return sum;
}
void cg_double(int *RowPtr,int *ColIdx,double *Val,
               double *x, double *b, int n, int *iter, int maxiter, double threshold)
{
    memset(x, 0, sizeof(double) * n);
    double *residual = (double *)malloc(sizeof(double) * n);
    double *y = (double *)malloc(sizeof(double) * n);
    double *p = (double *)malloc(sizeof(double) * n);
    double *q = (double *)malloc(sizeof(double) * n);
    *iter = 0;
    double norm = 0;
    double rho = 0;
    double rho_1 = 0;
    gemv_Handle_t temp = NULL;
    spmv_create_handle_all_in_one(&temp, n, RowPtr, ColIdx, Val, 8, Method_SellCSigma, sizeof(Val[0]), VECTOR_AVX512);
    // p0 = r0 = b - Ax0
    //matvec(A, x, y, n);
    spmv(temp,n,RowPtr,ColIdx,Val,x,y);
    for (int i = 0; i < n; i++)
        residual[i] = b[i] - y[i];
    //printvec(residual, n);

    do
    {
        //printf("\niter = %i\n", *iter);
        rho = dotproduct(residual, residual, n);
        if (*iter == 0)
        {
            for (int i = 0; i < n; i++)
                p[i] = residual[i];
        }
        else
        {
            double beta = rho / rho_1;
            for (int i = 0; i < n; i++)
                p[i] = residual[i] + beta * p[i];
        }

        //matvec(A, p, q, n);
        spmv(temp,n,RowPtr,ColIdx,Val,p,q);
        double alpha = rho / dotproduct(p, q, n);
        //printf("alpha = %f\n", alpha);
        for (int i = 0; i < n; i++)
            x[i] += alpha * p[i];
        for (int i = 0; i < n; i++)
            residual[i] += - alpha * q[i];

        rho_1 = rho;
        double error = vec2norm(residual, n); // / vec2norm(b, n);
        printf("error=%f\n",error);
        //printvec(x, n);
        *iter += 1;

        if (error < threshold)
            break;
    }
    while (*iter < maxiter);

    free(residual);
    free(y);
    free(p);
    free(q);
}
void new_cg_float(int *RowPtr,int *ColIdx,float *Val, float *x, float *b, int n, int *iter, int maxiter, double threshold)
{
    memset(x, 0, sizeof(float) * n);
    float *residual = (float *)malloc(sizeof(float) * n);
    float *y = (float *)malloc(sizeof(float) * n);
    float *p = (float *)malloc(sizeof(float) * n);
    float *q = (float *)malloc(sizeof(float) * n);
    *iter = 0;
    float norm = 0;
    float rho = 0;
    float rho_1 = 0;
    gemv_Handle_t temp = NULL;
    spmv_create_handle_all_in_one(&temp, n, RowPtr, ColIdx, Val, 8, Method_SellCSigma, sizeof(Val[0]), VECTOR_AVX512);

    // p0 = r0 = b - Ax0
    //matvec(A, x, y, n);
    spmv(temp,n,RowPtr,ColIdx,Val,x,y);
    for (int i = 0; i < n; i++)
        residual[i] = b[i] - y[i];
    //printvec(residual, n);

    do
    {
        //printf("\niter = %i\n", *iter);
        rho = dotproduct_float(residual, residual, n);
        if (*iter == 0)
        {
            for (int i = 0; i < n; i++)
                p[i] = residual[i];
        }
        else
        {
            double beta = rho / rho_1;
            for (int i = 0; i < n; i++)
                p[i] = residual[i] + beta * p[i];
        }

        //matvec(A, p, q, n);
        spmv(temp,n,RowPtr,ColIdx,Val,p,q);
        float alpha = rho / dotproduct_float(p, q, n);
        //printf("alpha = %f\n", alpha);
        for (int i = 0; i < n; i++)
            x[i] += alpha * p[i];
        for (int i = 0; i < n; i++)
            residual[i] += - alpha * q[i];

        rho_1 = rho;
        float error = vec2norm_float(residual, n); // / vec2norm(b, n);
        printf("error=%f\n",error);
        //printvec(x, n);
        *iter += 1;

        if (error < threshold)
            break;
    }
    while (*iter < maxiter);

    free(residual);
    free(y);
    free(p);
    free(q);
}
int main(int argc, char **argv)
{
    //int n;
    double *A, *x, *b;

    // method: gauss, lu_doolittle, cholesky
    printf("\n");
   
    char *filename = argv[1];
    printf ("filename = %s\n", filename);
    //read matrix_float
//    int m, n, nnzR, isSymmetric;
//    mmio_info(&m, &n, &nnzR, &isSymmetric, filename);
//    int *RowPtr = (int *) aligned_alloc(ALIGENED_SIZE,(m + 1) * sizeof(int));
//    int *ColIdx = (int *) aligned_alloc(ALIGENED_SIZE,nnzR * sizeof(int));
//    float *Val = (float *) aligned_alloc(ALIGENED_SIZE, nnzR * sizeof(float));
//    mmio_data(RowPtr, ColIdx, Val, filename);
//    float *X = (float *) aligned_alloc(ALIGENED_SIZE, sizeof(float) * (n));
//    float *Y = (float *) aligned_alloc(ALIGENED_SIZE, sizeof(float) * (m));
//    float *Y_golden = (float *) malloc(sizeof(float) * (m));
//    memset(X, 0, sizeof(float) * (n));
//    memset(Y, 0, sizeof(float) * (m));
//    memset(Y_golden, 0, sizeof(float) * (m));
//    for(int i = 0; i < n; i++)
//    X[i] = 1;
//    int iter=0;
//    for (int i = 0; i < m; i++)
//        for (int j = RowPtr[i]; j < RowPtr[i + 1]; j++)
//            Y_golden[i] += Val[j] * X[ColIdx[j]];
//        new_cg_float(RowPtr,ColIdx,Val,X,Y_golden,n,&iter,1000,0.00001);
//    printf("\n#iter of CG = %i\n", iter);
    //read_matrix_double
    int m, n, nnzR, isSymmetric;
    mmio_info(&m, &n, &nnzR, &isSymmetric, filename);
    int *RowPtr = (int *) aligned_alloc(ALIGENED_SIZE,(m + 1) * sizeof(int));
    int *ColIdx = (int *) aligned_alloc(ALIGENED_SIZE,nnzR * sizeof(int));
    double *Val = (double *) aligned_alloc(ALIGENED_SIZE, nnzR * sizeof(double));
    mmio_data(RowPtr, ColIdx, Val, filename);
    double *X = (double *) aligned_alloc(ALIGENED_SIZE, sizeof(double) * (n));
    double *Y = (double *) aligned_alloc(ALIGENED_SIZE, sizeof(double) * (m));
    double *Y_golden = (double *) malloc(sizeof(double) * (m));
    memset(X, 0, sizeof(double) * (n));
    memset(Y, 0, sizeof(double) * (m));
    memset(Y_golden, 0, sizeof(double) * (m));
    for(int i = 0; i < n; i++)
        X[i] = 1;
    int iter=0;
    for (int i = 0; i < m; i++)
        for (int j = RowPtr[i]; j < RowPtr[i + 1]; j++)
            Y_golden[i] += Val[j] * X[ColIdx[j]];
    cg_double(RowPtr,ColIdx,Val,X,Y_golden,n,&iter,1000,0.00001);
    printf("\n#iter of CG = %i\n", iter);
//    free(A);
//    free(x);
//    free(b);
}
