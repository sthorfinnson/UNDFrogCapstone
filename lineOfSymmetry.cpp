#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

Mat src; Mat src_gray; Mat newSrc;
int thresh = 100;
int max_thresh = 255;
RNG rng(12345);

double midpoint;

char **g_argv;

/// Function header
void thresh_callback(int, void* );

/** @function main */
int main( int argc, char** argv )
{
    
    g_argv=argv;
    /// Load source image and convert it to gray
    src = imread( argv[1], 1 );
    
    
    
    /// Convert image to gray and blur it
    cvtColor( src, src_gray, CV_BGR2GRAY );
    blur( src_gray, src_gray, Size(3,3) );
    
    
    thresh_callback( 0, 0 );
    
    
    
    waitKey(0);
    return(0);
}




/** @function thresh_callback */
void thresh_callback(int, void* )

{
    
    newSrc = imread(g_argv[1], 1);
    
    Mat canny_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    char* NewSource_window = "NewSource";
    
    
    
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
        double res= cv::norm(mc[i]-mc[i+1]);                    //Distance between two points is less than 60% of image width
        if(res < .6*cols){
            double res2 = cv::norm(mc[i].y - mc[i+1].y);        //Vertical distance between points is less than 10% of image height
            if(res2 < .1*rows){
                
                
                
                line( drawing, mc[i], mc[i+1],color, 1, 8, 0);
                line( newSrc, mc[i], mc[i+1],color, 1, 8, 0);       //Draw line horizontally between the biggest contours
                
                
                double slope_inverse = cv::norm((mc[i+1].x - mc[i].x)/(mc[i+1].y - mc[i].y));
                
                
                //printf("Slope Inverse %.2f\n\n", slope_inverse);
                
                
                
                
                
                
                double mY = cv::norm(mc[i].y+mc[i+1].y)/2;              //y cooredinate of midpoint between eyes
                //printf("mY %.2f\n\n", mY);
                double mX = cv::norm(mc[i].x+mc[i+1].x)/2;              //x coordinate of midpoint between eyes
                //printf("mX %.2f\n\n", mX);
                double b = mY-(slope_inverse*mX);                       //Creates ('b') point in y=mx+b for line of symmetry
                //printf("B %.2f\n\n", b);
                Point center(mX, mY);
                circle(newSrc, center, 10, 10);
                circle(newSrc, mc[i], 10, 10);
                circle(newSrc, mc[i+1], 10, 10);
                
                
                
                
                double midBottomX = (rows-b)/slope_inverse;             //Creates bottom x coordinate of line of symmetry based off ('b')
                
                double midTopX=-b/slope_inverse;                        //Creates top x coordinate of line of symmetry based off ('b')
                
                
                
                
                
                
                Point pt1(midBottomX, s.height);                        //Creates bottom point of line that crosses through midpoint of eyes
                Point pt2(midTopX, 0);                                  //Creates top point of line that crosses through midpoint of eyes
                //We could star the line at the midpoint of eyes if we choose...swap 0 for center.y
                
                
                
                Point leftEye(mc[i].x, mc[i].y);                         //Creates point of left eye
                Point rightEye(mc[i+1].x, mc[i+1].y);                    //Creates point of right eye
                
                double q = leftEye.y-(slope_inverse*leftEye.x);         //calculates ('b') in y=mx+b formula for left eye
                double g = rightEye.y-(slope_inverse*rightEye.x);       //calculates ('b') in y=mx+b formula for right eye
                
                double leftEyeX=(rows-q)/slope_inverse;                 //Calculates x coordinate for left eye line
                double rightEyeX=(rows-g)/slope_inverse;                //Calculates x coordinate for right eye line
                
                //double lDiffY = cv::norm(leftEye.y-rows);
                //double rDiffY = cv::norm(rightEye.y-rows);
                
                
                Point leftEyeBottom(leftEyeX, rows);                    //Creates end point for left eye line
                Point rightEyeBottom(rightEyeX, rows);                  //Creates end point for right eye line
                
                
                
                int res3 = cv::norm(center.x - cols/2);                 //Calculates horizontal difference from center of image
                res3 = abs(res3);
                if(res3 < .1*s.width){                                  //If the vertical line is less than 10% away from center of image
                    
                    
                    
                    
                    line( newSrc, pt1, pt2, color,2,8,0);                               //Draws line of symmetry
                    line( newSrc, leftEye, leftEyeBottom, color, 2, 8, 0);             //Draws line from left eye
                    line( newSrc, rightEye, rightEyeBottom, color,  2, 8, 0);         //Draws line from right eye
                    
                    
                    
                    
                    
                }
                
                
                
            }
            
            
        };
        
    }
    
    /// Show in a window
    namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
    imshow( "Contours", drawing );
    namedWindow( NewSource_window, CV_WINDOW_AUTOSIZE );
    imshow( NewSource_window, newSrc );
    createTrackbar( " Canny thresh:", "NewSource", &thresh, max_thresh, thresh_callback );
    
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