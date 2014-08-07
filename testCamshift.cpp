// TestCamshift.cpp : Defines the entry point for the console application.


#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <ctype.h>
#include "opencv2/opencv.hpp"  
#include "camshift.h"
#include "sqKmeans.h"

using namespace cv;
using namespace std;

vector<Rect> detectObject(Mat& img, CascadeClassifier& classifier, double scale);
Rect detectMaxObject(Mat& img, CascadeClassifier& classifier, double scale);

IplImage *image = 0, *hsv = 0, *hue = 0, *mask = 0, *backproject = 0, *histimg = 0;
CvHistogram *hist = 0;

int backproject_mode = 0;
int select_object = 0;
int track_object = 0;
int show_hist = 1;
CvPoint origin; 
CvRect selection;
CvRect track_window;
CvBox2D track_box;
CvConnectedComp track_comp;
int hdims = 16;
float hranges_arr[] = {0,180};
float* hranges = hranges_arr;
int vmin = 10, vmax = 256, smin = 30;

BaseMatrix<unsigned char>* cvtToBM(IplImage *img)
{
    BaseMatrix<unsigned char> *mat = new BaseMatrix<unsigned char>();
    mat->width = img->width;
    mat->height = img->height;
    mat->step = img->widthStep;
    mat->elements = (unsigned char*)img->imageData;
    if(mat->elements2d)
    {
        delete[] mat->elements2d;
    }
    mat->elements2d = new unsigned char*[mat->height];
    for(int i=0;i<mat->height;i++)
    {
        mat->elements2d[i] = (unsigned char*)img->imageData+i*img->widthStep;
    }
    return mat;
}


void myCamshift( IplImage *backproject, CvRect &track_window )
{
    BaseMatrix<unsigned char> *bp_mat;
    int width = backproject->width;
    int height = backproject->height;
    bp_mat = cvtToBM(backproject);
    Rectangle r;
    r.x = track_window.x;
    r.y = track_window.y;
    r.width = track_window.width;
    r.height = track_window.height;
    
    int ssss = camshift(bp_mat, r, 10, 1);
    //int ssss = meanshift(bp_mat, r, 10, 1);
    bp_mat->release();
    //printf("iter = %d\n", ssss);

    track_window.x = r.x;
    track_window.y = r.y;
    track_window.width = r.width;
    track_window.height = r.height;
}

void on_mouse( int event, int x, int y, int flags, void* param )
{
    if( !image )
        return;

    if( image->origin )
        y = image->height - y;

    if( select_object )
    {
        selection.x = MIN(x,origin.x);
        selection.y = MIN(y,origin.y);
        selection.width = selection.x + CV_IABS(x - origin.x);
        selection.height = selection.y + CV_IABS(y - origin.y);
        
        selection.x = MAX( selection.x, 0 );
        selection.y = MAX( selection.y, 0 );
        selection.width = MIN( selection.width, image->width );
        selection.height = MIN( selection.height, image->height );
        selection.width -= selection.x;
        selection.height -= selection.y;
    }

    switch( event )
    {
    case CV_EVENT_LBUTTONDOWN:
        origin = cvPoint(x,y);
        selection = cvRect(x,y,0,0);
        select_object = 1;
        break;
    case CV_EVENT_LBUTTONUP:
        select_object = 0;
        if( selection.width > 0 && selection.height > 0 )
            track_object = -1;
        break;
    }
}


CvScalar hsv2rgb( float hue )
{
    int rgb[3], p, sector;
    static const int sector_data[][3]=
        {{0,2,1}, {1,2,0}, {1,0,2}, {2,0,1}, {2,1,0}, {0,1,2}};
    hue *= 0.033333333333333333333333333333333f;
    sector = cvFloor(hue);
    p = cvRound(255*(hue - sector));
    p ^= sector & 1 ? 255 : 0;

    rgb[sector_data[sector][0]] = 255;
    rgb[sector_data[sector][1]] = 0;
    rgb[sector_data[sector][2]] = p;

    return cvScalar(rgb[2], rgb[1], rgb[0],0);
}

int main( int argc, char** argv )
{
    CvCapture* capture = 0;

	// hand detect related
    CascadeClassifier palmClassifier;
    bool flag = false;
    SqKmeans sq(10,8);
	palmClassifier.load("palm.xml");
    // hand detect related end

    if( argc == 1 || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0])))
        capture = cvCaptureFromCAM( argc == 2 ? argv[1][0] - '0' : 0 );
    else if( argc == 2 )
        capture = cvCaptureFromAVI( argv[1] ); 

    if( !capture )
    {
        fprintf(stderr,"Could not initialize capturing...\n");
        return -1;
    }

    printf( "Hot keys: \n"
        "\tESC - quit the program\n"
        "\tc - stop the tracking\n"
        "\tb - switch to/from backprojection view\n"
        "\th - show/hide object histogram\n"
        "To initialize tracking, select the object with mouse\n" );

    cvNamedWindow( "Histogram", 1 );
    cvNamedWindow( "CamShiftDemo", 1 );
    cvSetMouseCallback( "CamShiftDemo", on_mouse, 0 );
    cvCreateTrackbar( "Vmin", "CamShiftDemo", &vmin, 256, 0 );
    cvCreateTrackbar( "Vmax", "CamShiftDemo", &vmax, 256, 0 );
    cvCreateTrackbar( "Smin", "CamShiftDemo", &smin, 256, 0 );

    int count = 1;
    for(;;)
    {
        IplImage* frame = 0;
        int i, bin_w, c;

        frame = cvQueryFrame( capture );
        if( !frame )
            break;
        if( frame->origin == IPL_ORIGIN_TL )
            cvFlip( frame, NULL, 1);

        if( !image )
        {
            /* allocate all the buffers */
            image = cvCreateImage( cvGetSize(frame), 8, 3 );
            image->origin = frame->origin;
            hsv = cvCreateImage( cvGetSize(frame), 8, 3 );
            hue = cvCreateImage( cvGetSize(frame), 8, 1 );
            mask = cvCreateImage( cvGetSize(frame), 8, 1 );
            backproject = cvCreateImage( cvGetSize(frame), 8, 1 );
            hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );
            histimg = cvCreateImage( cvSize(320,200), 8, 3 );
            cvZero( histimg );
        }

        cvCopy( frame, image, 0 );

        cvCvtColor( image, hsv, CV_BGR2HSV );

	    //hand detect
        if(!flag)
        {
            Mat frameCopy = frame;
            Rect maxHand = detectMaxObject(frameCopy, palmClassifier, 1);
            if(maxHand.area()>0)
            {
                //cvRectangle(image, maxHand, CV_RGB(0,0,255), 3, 8, 0);
                if(sq.add(maxHand.x,maxHand.y,maxHand.width,maxHand.height))
                {
                    int x,y,w,h;
                    sq.getResult(x,y,w,h);

                    selection.x = MAX( x, 0 );
                    selection.y = MAX( y, 0 );
                    selection.width = MIN( w, image->width );
                    selection.height = MIN( h, image->height );

                    track_object = -1;

                    printf("+++++++++++++++ hand found\n");
                    flag = true;
                    sq.release();
                }
            }
        }

        if( track_object )
        {
            int _vmin = vmin, _vmax = vmax;

            cvInRangeS( hsv, cvScalar(0,smin,MIN(_vmin,_vmax),0),
                        cvScalar(180,256,MAX(_vmin,_vmax),0), mask );
            cvSplit( hsv, hue, 0, 0, 0 );

            if( track_object < 0 )
            {
                float max_val = 0.f;
                cvSetImageROI( hue, selection );
                cvSetImageROI( mask, selection );
                cvCalcHist( &hue, hist, 0, mask );
                cvGetMinMaxHistValue( hist, 0, &max_val, 0, 0 );
                cvConvertScale( hist->bins, hist->bins, max_val ? 255. / max_val : 0., 0 );
                cvResetImageROI( hue );
                cvResetImageROI( mask );
                track_window = selection;
                track_object = 1;

                cvZero( histimg );
                bin_w = histimg->width / hdims;
                for( i = 0; i < hdims; i++ )
                {
                    int val = cvRound( cvGetReal1D(hist->bins,i)*histimg->height/255 );
                    CvScalar color = hsv2rgb(i*180.f/hdims);
                    cvRectangle( histimg, cvPoint(i*bin_w,histimg->height),
                                 cvPoint((i+1)*bin_w,histimg->height - val),
                                 color, -1, 8, 0 );
                }
            }

            int min=256, max=-1;
            cvCalcBackProject( &hue, backproject, hist );
            cvAnd( backproject, mask, backproject, 0 );

            myCamshift( backproject, track_window );
                        
                       
            //cvCamShift( backproject, track_window,
            //            cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),
            //            &track_comp, &track_box );
            //cvMeanShift( backproject, track_window,
            //            cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),
            //            &track_comp);
            //track_window = track_comp.rect;
            
            if( backproject_mode )
                cvCvtColor( backproject, image, CV_GRAY2BGR );
            //if( image->origin )
            //    track_box.angle = -track_box.angle;
            //cvEllipseBox( image, track_box, CV_RGB(0,0,255), 3, CV_AA, 0 );
            //CvRect track_window1 = track_comp.rect;
            //cvRectangle(image, cvPoint(track_window1.x,track_window1.y),cvPoint(track_window1.x+track_window1.width,track_window1.y+track_window1.height), CV_RGB(255,0,0), 3, CV_AA, 0);
            cvRectangle(image, cvPoint(track_window.x,track_window.y),cvPoint(track_window.x+track_window.width,track_window.y+track_window.height), CV_RGB(255,0,0), 3, CV_AA, 0);
        }
        
        if( select_object && selection.width > 0 && selection.height > 0 )
        {
            cvSetImageROI( image, selection );
            cvXorS( image, cvScalarAll(255), image, 0 );
            cvResetImageROI( image );
        }

        cvShowImage( "CamShiftDemo", image );
        cvShowImage( "Histogram", histimg );

        c = cvWaitKey(1);
        if( (char) c == 27 )
            break;
        switch( (char) c )
        {
        case 'b':
            backproject_mode ^= 1;
            break;
        case 'c':
            track_object = 0;
            cvZero( histimg );
            break;
        case 'h':
            show_hist ^= 1;
            if( !show_hist )
                cvDestroyWindow( "Histogram" );
            else
                cvNamedWindow( "Histogram", 1 );
            break;
        default:
            ;
        }
    }

    cvReleaseCapture( &capture );
    cvDestroyWindow("CamShiftDemo");

    return 0;
}

// TestHandDetect.cpp : Defines the entry point for the console application.



vector<Rect> detectObject(Mat& img, CascadeClassifier& classifier, double scale)
{
	vector<Rect> objs;
	vector<Rect> objs_src;
	Mat gray, smallImg(cvRound(img.rows/scale), cvRound(img.cols/scale), CV_8UC1);
	cvtColor(img, gray, CV_BGR2GRAY);
	resize(gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR);
	equalizeHist(smallImg, smallImg);

	classifier.detectMultiScale(smallImg, objs, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT, Size(30,30));
				// CV_HAAR_FIND_BIGGEST_OBJECT
	for(vector<Rect>::const_iterator r=objs.begin(); r!=objs.end();r++)
	{
		Rect rect(cvRound(r->x*scale), cvRound(r->y*scale), cvRound(r->width*scale), cvRound(r->height*scale));
		objs_src.push_back(rect);
	}
	return objs_src;
}

Rect detectMaxObject(Mat& img, CascadeClassifier& classifier, double scale)
{
	vector<Rect> objs;
	vector<Rect> objs_src;
	Mat gray, smallImg(cvRound(img.rows/scale), cvRound(img.cols/scale), CV_8UC1);
	cvtColor(img, gray, CV_BGR2GRAY);
	resize(gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR);
	equalizeHist(smallImg, smallImg);

	classifier.detectMultiScale(smallImg, objs, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT, Size(30,30));
    if(objs.size()>0)
    {
        Rect r = objs[0];
	    Rect rect(cvRound(r.x*scale), cvRound(r.y*scale), cvRound(r.width*scale), cvRound(r.height*scale));
        return rect;
    }
	return Rect(0,0,0,0);
}
