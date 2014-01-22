// FrogMatcher.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "FrogMatcher.h"
#include "stdafx.h"
#include <iostream>
#include <vector>
#include <opencv2/calib3d/calib3d.hpp>
#include <fstream>
#include <string>
#include <cstdlib>
#include <windows.h>


using namespace std;
using namespace cv;


static void readme()
{
    cout << "\nThis enhanced image matching application was developed to match frogs from\n"
            "a frog identification database.  \n"
            "Design details to be added.\n"
            "Usage:\n"
            "./FrogMatcher.exe <path_to_folder>\n" << endl;
}

int main( int argc, char** argv )
//int main()
{
	// Application takes one argument of path to directory containing images.
	if( argc!=2 )
    {
        readme();
        return 1;
    }
    
	// Log activity to a file
	ofstream fout("matchReport.txt");
	fout << "Frog Image Match Report" << endl;		// Log file output
	
	string path = argv[1];
	string searchData = "\\Rs10*.jpg";
	string fullSearchPath = path + searchData;
	WIN32_FIND_DATA FindData;
	HANDLE hFind = FindFirstFile( (LPCSTR)fullSearchPath.c_str(), &FindData );
	fout << "Image directory: " + path << endl;		// Log file output
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
		string orgFile = myFileName;
		string filePath = path + "\\" + myFileName;

		// Log Image1, which is the frog to be matched
		fout << "Looking for a match to: " + myFileName << endl;					// Log file output

		Mat image1 = imread( filePath, 0 );
		if( image1.empty() )
		{
			cout << "\n Error, couldn't read image filename " << filePath << endl;
			return 1;
		}
        
		// Loop through directory to test files against the outer loop one by one
		string searchData2 = "\\Rs09*.jpg";
		string fullSearchPath = path + searchData2;
		WIN32_FIND_DATA FindData2;
		HANDLE hFind2 = FindFirstFile( (LPCSTR)fullSearchPath.c_str(), &FindData2 );

		do
		{
			string myFileName2 = FindData2.cFileName;
			string orgFile2 = myFileName2;
			if (myFileName2==myFileName)
				fout << "Skipping self check" << endl;
			else
			{
			string filePath2 = path + "\\" + myFileName2;

			cv::Mat image2= cv::imread( filePath2, 0 );
		
			// Read input images
			//cv::Mat image2= cv::imread("h:/TrainingSet/Rs03-270.jpg",0);
			//cv::Mat image1= cv::imread("h:/TrainingSet/Rs02-297.jpg",0);
			//if( argc!=3 )
			//{
			//	return 0;
			//}
			//string image1file = argv[1];
			//string image2file = argv[2];
			//cv::Mat image1= cv::imread(image1file,0);
			//cv::Mat image2= cv::imread(image2file,0);
			if (!image1.data || !image2.data)
					return 0; 

			// Display the images
			//cv::namedWindow("Right Image");
			//cv::imshow("Right Image",image1);
			//cv::namedWindow("Left Image");
			//cv::imshow("Left Image",image2);

			// Prepare the matcher
			FrogMatcher fmatcher;
			fmatcher.setConfidenceLevel(0.98);
			fmatcher.setMinDistanceToEpipolar(1.0);
			fmatcher.setRatio(0.65f);
			cv::Ptr<cv::FeatureDetector> pfd= new cv::SurfFeatureDetector(10); 
			fmatcher.setFeatureDetector(pfd);

			// Match the two images
            std::cout << myFileName << " : " << myFileName2 << std::endl;
			std::vector<cv::DMatch> matches;
			std::vector<cv::KeyPoint> keypoints1, keypoints2;
			cv::Mat fundemental= fmatcher.match(image1,image2,matches, keypoints1, keypoints2);

			if (matches.size()>0) {

				// draw the matches
				cv::Mat imageMatches;
				cv::drawMatches(image1,keypoints1,  // 1st image and its keypoints
									image2,keypoints2,  // 2nd image and its keypoints
												matches,                        // the matches
												imageMatches,           // the image produced
												cv::Scalar(255,255,255)); // color of the lines
				//cv::namedWindow("Matches");
				//cv::imshow("Matches",imageMatches);
        
				// Convert keypoints into Point2f       
				std::vector<cv::Point2f> points1, points2;
        
				for (std::vector<cv::DMatch>::const_iterator it= matches.begin();
								 it!= matches.end(); ++it) {

								 // Get the position of left keypoints
								 float x= keypoints1[it->queryIdx].pt.x;
								 float y= keypoints1[it->queryIdx].pt.y;
								 points1.push_back(cv::Point2f(x,y));
								 cv::circle(image1,cv::Point(x,y),3,cv::Scalar(255,255,255),3);
								 // Get the position of right keypoints
								 x= keypoints2[it->trainIdx].pt.x;
								 y= keypoints2[it->trainIdx].pt.y;
								 cv::circle(image2,cv::Point(x,y),3,cv::Scalar(255,255,255),3);
								 points2.push_back(cv::Point2f(x,y));
				}
        
				// Draw the epipolar lines
				std::vector<cv::Vec3f> lines1; 
				cv::computeCorrespondEpilines(cv::Mat(points1),1,fundemental,lines1);
                
				for (std::vector<cv::Vec3f>::const_iterator it= lines1.begin();
								 it!=lines1.end(); ++it) {

								 cv::line(image2,cv::Point(0,-(*it)[2]/(*it)[1]),
													 cv::Point(image2.cols,-((*it)[2]+(*it)[0]*image2.cols)/(*it)[1]),
																 cv::Scalar(255,255,255));
				}

				std::vector<cv::Vec3f> lines2; 
				cv::computeCorrespondEpilines(cv::Mat(points2),2,fundemental,lines2);
        
				for (std::vector<cv::Vec3f>::const_iterator it= lines2.begin();
							 it!=lines2.end(); ++it) {

								 cv::line(image1,cv::Point(0,-(*it)[2]/(*it)[1]),
													 cv::Point(image1.cols,-((*it)[2]+(*it)[0]*image1.cols)/(*it)[1]),
																 cv::Scalar(255,255,255));
				}

				// Display the images with epipolar lines
				//cv::namedWindow("Left Image Epilines (RANSAC)");
				//cv::imshow("Left Image Epilines (RANSAC)",image1);
				//cv::namedWindow("Right Image Epilines (RANSAC)");
				//cv::imshow("Right Image Epilines (RANSAC)",image2);

				// Combine the epipolar lines images
				int rows = image1.rows;
				int cols = image1.cols;
				Mat combinedImage = cvCreateMat(MAX(image1.rows,image2.rows), image1.cols + image2.cols, image1.type());
				cv::Mat tmp = combinedImage(cv::Rect(0, 0, image1.cols, image1.rows));
				image1.copyTo(tmp);
				tmp = combinedImage(cv::Rect(image1.cols, 0, image2.cols, MAX(image1.rows,image2.rows)));
				image2.copyTo(tmp);
				//cv::namedWindow("Combined Epilines Images");
				//cv::imshow("Combined Epilines Images",combinedImage);

				// Output the combined image (epipolar lines matches)
				string outputFile = ".\\Output\\" + myFileName + ".matches." + myFileName2;
				string outimFile = ".\\Output\\" + myFileName + ".im." + myFileName2;
				cv::imwrite(outputFile, combinedImage);
				cv::imwrite(outimFile, imageMatches);
				fout << myFileName + ", " + myFileName2 + ", MATCH" << endl;
				//cv::waitKey();
				//return 0;
			}
			else
				fout << myFileName + ", " + myFileName2 + ", 0" << endl;
				//return 0;
			}
		}
		// Next iteration
		while( FindNextFile(hFind2, &FindData2 ) >0 );

	}

	// Next iteration
	while( FindNextFile(hFind, &FindData ) >0 );

	// Exit application
	fout.close();
	return 0;
}
