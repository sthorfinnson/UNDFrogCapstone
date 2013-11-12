#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

Mat src; Mat src_gray;
int thresh = 100;
int max_thresh = 255;
RNG rng(12345);

/// Function header
void thresh_callback(int, void* );

/** @function main */
int main( int argc, char** argv )
{
    /// Load source image and convert it to gray
    src = imread( argv[1], 1 );
    
    /// Convert image to gray and blur it
    cvtColor( src, src_gray, CV_BGR2GRAY );
    blur( src_gray, src_gray, Size(3,3) );
    
    /// Create Window
    char* source_window = "Source";
    namedWindow( source_window, CV_WINDOW_AUTOSIZE );
    imshow( source_window, src );
    
    createTrackbar( " Canny thresh:", "Source", &thresh, max_thresh, thresh_callback );
    thresh_callback( 0, 0 );
    
    waitKey(0);
    return(0);
}



/** @function thresh_callback */
void thresh_callback(int, void* )
{
    Mat canny_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    
    
    int rows = src.rows;
    int cols = src.cols;
    
    cv::Size s = src.size();
    rows = s.height;
    cols = s.width;
    int middle = cols/2;
    
    /// Detect edges using canny
    Canny( src_gray, canny_output, thresh, thresh*2, 3 );
    /// Find contours
    findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    
    /// Get the moments
    vector<Moments> mu(contours.size() );
    for( int i = 0; i < contours.size(); i++ )
    { mu[i] = moments( contours[i], false ); }
    
    ///  Get the mass centers:
    vector<Point2f> mc( contours.size() );
    for( int i = 0; i < contours.size(); i++ )
    { mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 );
        
    }
    
    
    /// Draw contours
    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    for( int i = 0; i< contours.size(); i++ )
    {
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() );
        circle( drawing, mc[i], 4, color, -1, 8, 0 );
        double res= cv::norm(mc[i]-mc[i+1]);                    //Distance between two points is less than 300 pixels
        if(res < 300){
            double res2 = cv::norm(mc[i].y - mc[i+1].y);        //Vertical distance between points is less than 10 pixels
            if(res2 < 10){
                
                
            
                line( drawing, mc[i], mc[i+1],color, 1, 8, 0);  //Draw line horizontally between the biggest contours
                Point pt3(((mc[i+1].x - mc[i].x)/2)+mc[i].x, 0);
                Point pt4(((mc[i+1].x - mc[i].x)/2)+mc[i].x,s.height);
                int res3 = cv::norm(pt4.x - middle);
                res3 = abs(res3);
                printf("Res 3 x\t%.2d\n", res3);
                printf("pt4.x %d\n", pt4.x);
                printf("pt3.x %d\n", pt3.x);
                printf("middle %d\n", middle);
                if(res3 < 50){                                  //If the vertical line is less than 50 pixels away from midpoint of image--Draw
                    line( drawing, pt3, pt4, color,1,8,0);
                    printf("First x\t%.2f\n", mc[i].x);
                    printf("First y\t%.2f\n", mc[i].y);
                    printf("Second x\t%.2f\n", mc[i+1].x);
                    printf("Second y\t%.2f\n", mc[i+1].y);
                }
                
                
            
            }
            
            
        };
        
    }
    
    /// Show in a window
    namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
    imshow( "Contours", drawing );
    
    /// Calculate the area with the moments 00 and compare with the result of the OpenCV function
    printf("\t Info: Area and Contour Length \n");
    for( int i = 0; i< contours.size(); i++ )
    {
       // printf(" * Contour[%d] - Area (M_00) = %.2f - Area OpenCV: %.2f - Length: %.2f \n", i, mu[i].m00, contourArea(contours[i]), arcLength( contours[i], true ) );
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() );
        circle( drawing, mc[i], 4, color, -1, 8, 0 );
    }
}
