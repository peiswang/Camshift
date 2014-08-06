#ifndef __UTILS_H__
#define __UTILS_H__

#include "matrix.h"
#include "basic.h"

#include <stdio.h>
#include <string.h>

#define max(x,y) (x)>(y)?(x):(y)
#define min(x,y) (x)<(y)?(x):(y)

template<typename T>
Moments calcMoments( BaseMatrix<T> *mat, int rank )
{
    Moments moments;
    //memset(&moments, 0, sizeof(Moments));
    moments.m00 = moments.m10 = moments.m01 = 0;
    moments.m20 = moments.m11 = moments.m02 = 0;

    if(rank>2)
    {
        printf("warn: rank > 2! \n");
        return moments;
    }

    int width = mat->width;
    int height = mat->height;

    double *v = new double[width];
    double *h = new double[height];
    double tmp = 0.0;

    for( int i=0;i<width;i++ )
    {
        v[i] = 0;
        for( int j=0;j<height;j++ )
        {
            v[i] += (*mat)(j,i);
        }
    }

    for( int i=0;i<height;i++ )
    { 
        h[i] = 0;
        tmp = 0.0;
        for( int j=0;j<width;j++ )
        {
            h[i] += (*mat)(i,j);
            tmp += (*mat)(i,j)*j;
        }
        moments.m11 += tmp * i;
    }
    for( int i=0;i<width;i++ )
    {
        moments.m00 += v[i];
        tmp = v[i] * i; 
        moments.m10 += tmp;
        moments.m20 += tmp * i;
    }
    //printf("max = %d,width = %d, height = %d, m00 is %lf\n", max, width, height, moments.m00);
    for( int i=0;i<height;i++ )
    {
        tmp = h[i] * i;
        moments.m01 += tmp;
        moments.m02 += tmp * i;
    }
    moments.inv_m00 = 1.0 / moments.m00;
    double x_ = moments.m10 * moments.inv_m00;
    double y_ = moments.m01 * moments.inv_m00;
    moments.mu20 = moments.m20 - x_ * moments.m10;
    moments.mu11 = moments.m11 - x_ * moments.m01;
    moments.mu02 = moments.m02 - y_ * moments.m01;
    delete[] v;
    delete[] h;
    return moments;
}

#endif
