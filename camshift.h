
#ifndef __CAMSHIFT_H__
#define __CAMSHIFT_H__

#include "matrix.h"

int meanshift( BaseMatrix<unsigned char> *proMat, Rectangle &window, int max_iter, double epsilon );
int camshift( BaseMatrix<unsigned char> *proMat, Rectangle &window, int max_iter, double epsilon );

#endif
