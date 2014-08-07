#include <stdio.h>
#include <float.h>
#include <math.h>
#include "utils.h"
#include "matrix.h"
#include "camshift.h"

int meanshift( BaseMatrix<unsigned char> *proMat, Rectangle &win_rect, int max_iter, double epsilon )
{
    double eps = epsilon * epsilon;
    BaseMatrix<unsigned char> win_mat;
    Moments moments;
    double inv_m00 = 0.0;
    int dx, dy, nx, ny;
    int i;
    for( i=0;i<max_iter;i++ )
    {
        proMat->getSubRect( &win_mat, win_rect );
        moments = calcMoments( &win_mat, 2 );

        if(fabs(moments.m00) < DBL_EPSILON)
            break;
        inv_m00 = moments.inv_m00;
        dx = round(moments.m10*inv_m00 - win_rect.width*0.5);
        dy = round(moments.m01*inv_m00 - win_rect.height*0.5);

        //printf("m10 = %f; m01 = %f\n", moments.m10, moments.m01);

        nx = win_rect.x + dx;
        ny = win_rect.y + dy;
        
        if(nx<0) 
            nx = 0;
        else if(nx+win_rect.width>proMat->width)
            nx = proMat->width - win_rect.width;

        if(ny<0)
            ny = 0;
        else if(ny+win_rect.height>proMat->height)
            ny = proMat->height - win_rect.height;

        dx = nx - win_rect.x;
        dy = ny - win_rect.y;
        win_rect.x = nx;
        win_rect.y = ny;

        if(dx*dx+dy*dy<eps)
            break;
    }
    return i;
}

int camshift( BaseMatrix<unsigned char> *proMat, Rectangle &win_rect, int max_iter, double epsilon )
{
    const int TOLERANCE = 15;
    int iter_num = 0;
    double m00, m10, m01, mu11, mu20, mu02, inv_m00;
    double a, b, c, xc, yc;
    double square, theta, cs, sn, rotate_a, rotate_c, length, width;
    Moments moments;
    BaseMatrix<unsigned char> win_mat;
    iter_num = meanshift( proMat, win_rect, max_iter, epsilon );
    
    win_rect.x -= TOLERANCE;
    if( win_rect.x < 0 )
        win_rect.x = 0;
    
    win_rect.y -= TOLERANCE;
    if( win_rect.y < 0 )
        win_rect.y = 0;

    win_rect.width += 2*TOLERANCE;
    if( win_rect.x+win_rect.width > proMat->width )
        win_rect.width = proMat->width - win_rect.x;

    win_rect.height+= 2*TOLERANCE;
    if( win_rect.y+win_rect.height> proMat->height)
        win_rect.height = proMat->height - win_rect.y;


    proMat->getSubRect( &win_mat, win_rect );
    moments = calcMoments( &win_mat, 2 );
    m00 = moments.m00;
    m01 = moments.m01; 
    m10 = moments.m10;
    mu20 = moments.mu20;
    mu11 = moments.mu11;
    mu02 = moments.mu02;
    inv_m00 = moments.inv_m00;

    if(fabs(m00) < DBL_EPSILON)
        return -1;
    
    xc = round(m10 * inv_m00 + win_rect.x);
    yc = round(m01 * inv_m00 + win_rect.y);

    a = mu20 * inv_m00;
    b = mu11 * inv_m00;
    c = mu02 * inv_m00;

    square = sqrt(4*b*b+(a-c)*(a-c));
    theta = atan2(2*b, a-c+square);

    cs = cos(theta);
    sn = sin(theta);

    rotate_a = cs*cs*mu20 + 2*cs*sn*mu11 + sn*sn*mu02;
    rotate_c = sn*sn*mu20 - 2*cs*sn*mu11 + cs*cs*mu02;
    length = sqrt(rotate_a*inv_m00)*4;
    width = sqrt(rotate_c*inv_m00)*4;

    if(length<width)
    {
        double tmp = length;
        length = width;
        width = tmp;

        tmp = cs;
        cs = sn;
        sn = tmp;

        theta = PI * 0.5 - theta;
    }

    int t0, t1;
    int _xc = round(xc);
    int _yc = round(yc);

    t0 = round(fabs(length*cs));
    t1 = round(fabs(width*sn));
    t0 = max(t0, t1) + 2;
    win_rect.width = min(t0, (proMat->width-_xc)*2);

    t0 = round(fabs(length*sn));
    t1 = round(fabs(width*cs));
    t0 = max(t0, t1) + 2;
    win_rect.height = min(t0, (proMat->height-_yc)*2);

    win_rect.x = max(0, _xc - win_rect.width*0.5);
    win_rect.y = max(0, _yc - win_rect.height*0.5);

    win_rect.width = min(proMat->width-win_rect.x, win_rect.width);
    win_rect.height = min(proMat->height-win_rect.y, win_rect.height);

    return iter_num;
}
