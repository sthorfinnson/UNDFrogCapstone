#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <opencv/highgui.h>
#include "opencv2/core/core.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;

void readme();

/** @function main */
int main( int argc, char** argv )
{
    if( argc != 4 )
    { readme(); return -1; }

    Mat image = imread( argv[1], CV_LOAD_IMAGE_COLOR );

    /** Converting the input from the char argv to ints using atoi */
    int boxX = atoi(argv[2]); int boxY = atoi(argv[3]);

    /** Check if image data is present */
    if( !image.data )
    { std::cout<< " --(!) Error reading images " << std::endl; return -1; }

    /** Finds the max dimensions of the image*/
    int maxX = image.size().width; int maxY = image.size().height;

    /** Dividing the image into sections */
    /** Also assigns the number of 'black' pixels to an array */
    Vec3f bgrPixel; float blue, green, red;
    int y1 = 0, x1 = 0; int y2 = boxY; int x2 = boxX;
    int xDiv = maxX / boxX; int yDiv = maxY / boxY;
    int xRmdr = maxX % boxX; int yRmdr = maxY % boxY;
    double blkPxls [yDiv][xDiv];
    for (int yCV = 0; yCV < yDiv; yCV++)
    {
        for (int xCV = 0; xCV < xDiv; xCV++)
        {
            double blkCV = 0;
            for( int y = y1; y < y2; y++ )
            {
                for( int x = x1; x < x2; x++)
                {
                    bgrPixel = image.at<Vec3b>(y, x);
    	    		blue = bgrPixel.val[0]; green = bgrPixel.val[1]; red = bgrPixel.val[2];
			    	if( (blue < 20) && (green < 20) && (red < 20) ) //now 20,20,20 (92.16% BLK) <= was 10,10,10 (94.51% BLK)
			    	{ blkCV++;}
                }
            }
            blkPxls[yCV][xCV] = blkCV;
            x1 += boxX; x2 += boxX;
         }
         y1 += boxY; y2 += boxY;
    }

    /** Checking the left over sections **
    // * To be fixed later *
    double blkCV = 0;
    for(int y = y2; y =< yRmdr; y++)
    {
        for(int x = x2; x =< xRmdr; x++)
        {
            bgrPixel = image.at<Vec3b>(y, x);
            blue = bgrPixel.val[0]; green = bgrPixel.val[1]; red = bgrPixel.val[2];
            if( (blue < 20) && (green < 20) && (red < 20) )
            { blkCV++;}
        }
    } */

    /** Displaying Percentage data */
    double percBlkPxls;
    std::cout << "Percentile blk pixels for the array" << std::endl;
    for(int y = 0; y <= yDiv; y++)
    {
        for(int x = 0; x <= xDiv; x++)
        {
            percBlkPxls = ( blkPxls[y][x] / (boxX * boxY) ) * 100;
            std::cout.precision(1);
            std::cout << " "  << std::fixed << percBlkPxls  << " ";
        }
        std::cout << std::endl;
    }

    /**Retained in the hope of later exploits**
    namedWindow( "Display Image", CV_WINDOW_AUTOSIZE );
    imshow( "Display Image", image );

    waitKey(0);
    */
 
    return 0;
}

/** @function readme */
void readme() { 
    std::cout << " Usage: ./findBlkPxls <img1> <boxX> <boxY>" << std::endl;
}
