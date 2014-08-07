#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <assert.h>
#include <stdio.h>
#include "basic.h"

template <typename T>
class BaseMatrix
{
public:
    BaseMatrix()
    { 
        allocated = false; 
        rows = cols = step = 0;
        size = 0;
        elements = 0;
        elements2d = 0;
    }
    ~BaseMatrix()
    {
        release();
    }

    void release()
    {
        if(elements2d)
        {
            delete[] elements2d;
        }
        if(allocated && elements)
        {
            delete[] elements;
        }
    }

    bool init( int rows_, int cols_ )
    {
        rows = rows_;
        cols = cols_;
        step = cols;
        elements = new T[rows*cols];
        size = rows*cols;
        elements2d = new T*[rows];
        for( int i=0;i<rows;i++ )
        {
            elements2d[i] = elements+i*step;
        }
        allocated = true;
    }

    inline T& operator()( int index ) const
    {
        assert(index>=0 && index<size);
        return elements[index];
    }

    inline T& operator()( int idr, int idc ) const
    {
        assert( idr>=0 && idr<rows && idc>=0 && idc<cols );
        //return elements[idr*step+idc];
        return elements2d[idr][idc];
    }

    void getSubRect( BaseMatrix<T> *mat, Rectangle r )
    {
        r.x = r.x<0 ? 0 : r.x>=cols ? cols-1 : r.x;
        r.y = r.y<0 ? 0 : r.y>=rows ? rows-1 : r.y;
        r.width = r.width<0 ? 0 : r.x+r.width>=cols ? cols-r.x-1 : r.width;
        r.height = r.height<0 ? 0 : r.y+r.height>=rows? rows-r.y-1 : r.height;

        mat->elements = elements + r.y * step + r.x;
        mat->step = step;
        mat->width = r.width;
        mat->height = r.height;
        
        if(mat->elements2d)
        {
            delete[] mat->elements2d;
        }
        mat->elements2d = new T*[mat->rows];

        for(int i=0;i<mat->rows;i++)
        {
            mat->elements2d[i] = elements2d[r.y+i]+r.x;
            //mat->elements2d[i] = mat->elements + i*mat->step;
        }

    }

    void visit( void(*p)(T &t) )
    {
        for(int i=0;i<size;i++)
        {
            (*p)(elements[i]);
        }
    }

    void visit2d( void(*p)(T &t) )
    {
        for(int i=0;i<rows;i++)
        {
            for(int j=0;j<cols;j++)
            {
                (*p)(elements2d[i][j]);
            }
        }
    }

    void print( void(*p)(T &t) )
    {
        for(int i=0;i<rows;i++)
        {
            for(int j=0;j<cols;j++)
            {
                (*p)(elements2d[i][j]);
            }
            printf("\n");
        }
    }


    union
    {
        int height;
        int rows;
    };
    union
    {
        int width;
        int cols;
    };

    int step;

    T *elements;
    T **elements2d;
    
    bool allocated;
    int size;

};


#endif
