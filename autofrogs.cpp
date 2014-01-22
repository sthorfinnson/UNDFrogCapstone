// AutoFrogs.cpp : Defines the entry point for the console application.
// 
// Frog Capstone Project - UND Fall 2013
// Frog extraction routine using grabcut
// Author: David Carson
//
// Description:  This program uses the grabcut algorithm to extract frogs from the Frog ID image set
//				 and save them in a new directory "extractedFrogs".

#include "stdafx.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <windows.h>


using namespace std;
using namespace cv;

string orgFile;

static void help()
{
    cout << "\nThis utilizes GrabCut segmentation to extract frogs from digital photograph\n"
            "images.  It will iterate through the images in the target directory, extract\n"
            "the frog by masking the background, and save the new image to the newly created\n"
            "directory 'ExtractedFrogs' located with this executable.\n"
            "Call:\n"
            "./AutoFrogs.exe <path_to_folder>\n"
        "\nSelect a rectangular area around the object you want to segment\n" << endl;
}

const Scalar RED = Scalar(0,0,255);
const Scalar PINK = Scalar(230,130,255);
const Scalar BLUE = Scalar(255,0,0);
const Scalar LIGHTBLUE = Scalar(255,255,160);
const Scalar GREEN = Scalar(0,255,0);

const int BGD_KEY = CV_EVENT_FLAG_CTRLKEY;
const int FGD_KEY = CV_EVENT_FLAG_SHIFTKEY;

static void getBinMask( const Mat& comMask, Mat& binMask )
{
    if( comMask.empty() || comMask.type()!=CV_8UC1 )
        CV_Error( CV_StsBadArg, "comMask is empty or has incorrect type (not CV_8UC1)" );
    if( binMask.empty() || binMask.rows!=comMask.rows || binMask.cols!=comMask.cols )
        binMask.create( comMask.size(), CV_8UC1 );
    binMask = comMask & 1;
}

class GCApplication
{
public:
    enum{ NOT_SET = 0, IN_PROCESS = 1, SET = 2 };
    static const int radius = 2;
    static const int thickness = -1;

    void reset();
	void save();
    void setImageAndWinName( const Mat& _image, const string& _winName );
    void showImage() const;
    int nextIter();
	void setRect();
    int getIterCount() const { return iterCount; }
private:
    void setRectInMask();
    void setLblsInMask( int flags, Point p, bool isPr );

    const string* winName;
    const Mat* image;
    Mat mask;
    Mat bgdModel, fgdModel;

    uchar rectState, lblsState, prLblsState;
    bool isInitialized;

    Rect rect;
    vector<Point> fgdPxls, bgdPxls, prFgdPxls, prBgdPxls;
    int iterCount;
};

void GCApplication::reset()
{
    if( !mask.empty() )
        mask.setTo(Scalar::all(GC_BGD));
    bgdPxls.clear(); fgdPxls.clear();
    prBgdPxls.clear();  prFgdPxls.clear();

    isInitialized = false;
    rectState = NOT_SET;
    lblsState = NOT_SET;
    prLblsState = NOT_SET;
    iterCount = 0;
}

// Todo:  implement save function and remove imwrite from showImage function.
void GCApplication::save()
{
    Mat res;
	image->copyTo( res );

	imwrite("frogcut.jpg", res);
}

void GCApplication::setImageAndWinName( const Mat& _image, const string& _winName  )
{
    if( _image.empty() || _winName.empty() )
        return;
    image = &_image;
    winName = &_winName;
    mask.create( image->size(), CV_8UC1);
    reset();
}

void GCApplication::showImage() const
{
    if( image->empty() || winName->empty() )
        return;

    Mat res;
    Mat binMask;
    if( !isInitialized )
        image->copyTo( res );
    else
    {
        getBinMask( mask, binMask );
        image->copyTo( res, binMask );
    }

    vector<Point>::const_iterator it;
    for( it = bgdPxls.begin(); it != bgdPxls.end(); ++it )
        circle( res, *it, radius, BLUE, thickness );
    for( it = fgdPxls.begin(); it != fgdPxls.end(); ++it )
        circle( res, *it, radius, RED, thickness );
    for( it = prBgdPxls.begin(); it != prBgdPxls.end(); ++it )
        circle( res, *it, radius, LIGHTBLUE, thickness );
    for( it = prFgdPxls.begin(); it != prFgdPxls.end(); ++it )
        circle( res, *it, radius, PINK, thickness );

    if( rectState == IN_PROCESS || rectState == SET )
        rectangle( res, Point( rect.x, rect.y ), Point(rect.x + rect.width, rect.y + rect.height ), GREEN, 2);

    imshow( *winName, res );
	//string extractedImage = "extracted_" + gcapp.orgFile;
	imwrite(orgFile, res);
}

void GCApplication::setRectInMask()
{
    assert( !mask.empty() );
    mask.setTo( GC_BGD );
    rect.x = max(0, rect.x);
    rect.y = max(0, rect.y);
    rect.width = min(rect.width, image->cols-rect.x);
    rect.height = min(rect.height, image->rows-rect.y);
    (mask(rect)).setTo( Scalar(GC_PR_FGD) );
}

void GCApplication::setLblsInMask( int flags, Point p, bool isPr )
{
    vector<Point> *bpxls, *fpxls;
    uchar bvalue, fvalue;
    if( !isPr )
    {
        bpxls = &bgdPxls;
        fpxls = &fgdPxls;
        bvalue = GC_BGD;
        fvalue = GC_FGD;
    }
    else
    {
        bpxls = &prBgdPxls;
        fpxls = &prFgdPxls;
        bvalue = GC_PR_BGD;
        fvalue = GC_PR_FGD;
    }
    if( flags & BGD_KEY )
    {
        bpxls->push_back(p);
        circle( mask, p, radius, bvalue, thickness );
    }
    if( flags & FGD_KEY )
    {
        fpxls->push_back(p);
        circle( mask, p, radius, fvalue, thickness );
    }
}

void GCApplication::setRect()
{
    // TODO add bad args check
	int width = image->cols;
	int height = image->rows;

	rect = Rect( Point(10, 10), Point(width-10, height-10) );
    rectState = SET;
    setRectInMask();
    assert( bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty() );
    showImage();

}


int GCApplication::nextIter()
{
    if( isInitialized )
        grabCut( *image, mask, rect, bgdModel, fgdModel, 1 );
    else
    {
        if( rectState != SET )
            return iterCount;

        if( lblsState == SET || prLblsState == SET )
            grabCut( *image, mask, rect, bgdModel, fgdModel, 1, GC_INIT_WITH_MASK );
        else
            grabCut( *image, mask, rect, bgdModel, fgdModel, 1, GC_INIT_WITH_RECT );

        isInitialized = true;
    }
    iterCount++;

    bgdPxls.clear(); fgdPxls.clear();
    prBgdPxls.clear(); prFgdPxls.clear();

    return iterCount;
}

GCApplication gcapp;


int main( int argc, char** argv )
{
	// Application takes one argument of path to directory containing images.
	if( argc!=2 )
    {
        help();
        return 1;
    }
    string path = argv[1];
	string searchData = "\\*.jpg";
	string fullSearchPath = path + searchData;
	WIN32_FIND_DATA FindData;
	HANDLE hFind = FindFirstFile( (LPCSTR)fullSearchPath.c_str(), &FindData );

	if (hFind == INVALID_HANDLE_VALUE)
	{
		cout << "Path is " << path;
		cout << "\nfullSearchPath is " << fullSearchPath << "\n";
		cout << "Error(" << errno << ") opening " << path << endl;
		return errno;
	}

	// Iterate through the images
	do
	{
		string myFileName = FindData.cFileName;
		orgFile = myFileName;
		string filePath = path + "\\" + myFileName;

		Mat image = imread( filePath, 1 );
		if( image.empty() )
		{
			cout << "\n Error, couldn't read image filename " << filePath << endl;
			return 1;
		}

		// Carry over from manual app
		//help();

		const string winName = "Current Frog Image";
		cvNamedWindow( winName.c_str(), CV_WINDOW_AUTOSIZE );
		//cvSetMouseCallback( winName.c_str(), on_mouse, 0 );

		gcapp.setImageAndWinName( image, winName );

		for(;;)
		{
			// Set rect for grabcut
			gcapp.setRect();
			// Only doing one iteration per image - but code is ready to implement several
			int iterCount = gcapp.getIterCount();
			cout << "<" << iterCount << "... ";
			int newIterCount = gcapp.nextIter();
			if( newIterCount > iterCount )
			{
				gcapp.showImage();
				gcapp.save();
				cout << iterCount << ">" << endl;
			}
			else
				cout << "rect must be determined>" << endl;
			break;
			//}
		}

	}
	// Next iteration
	while( FindNextFile(hFind, &FindData ) >0 );

	// Exit application
	return 0;
}
