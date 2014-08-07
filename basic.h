#ifndef __BASIC_H__
#define __BASIC_H__

#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif
/**
 * Basic Structs used by Matrix
 * */
typedef struct
{
    int x;
    int y;
    int width;
    int height;
} Rectangle;

typedef struct
{
    unsigned char v1;
    unsigned char v2;
    unsigned char v3;
} Vector3uc;

typedef struct 
{
    int v1;
    int v2;
    int v3;
} Vector3i;

typedef struct
{
    float v1;
    float v2;
    float v3;
} Vector3f;

typedef struct
{
    double m00;
    double m10;
    double m01;
    double m20;
    double m11;
    double m02;
    double m30;
    double m21;
    double m12;
    double m03;
    double mu20;
    double mu11;
    double mu02;
    double inv_m00;
} Moments;

#endif
