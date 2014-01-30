// FrogMatcher.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "FrogMatcher.h"
#include <iostream>
#include <vector>
#include <opencv2/calib3d/calib3d.hpp>
#include <fstream>
#include <string>
#include <cstdlib>
#include <windows.h>
#include <ctime>


using namespace std;
using namespace cv;
using namespace cv::gpu;


// ##########################################################################################
// Readme instructions for command-line 
// ##########################################################################################
static void readme()
{
    cout << "\nThis enhanced image matching application was developed to match frogs from\n"
            "a frog identification database.  \n"
            "Design details to be added.\n"
            "Usage:\n"
            "./FrogMatcher.exe <path_to_folder>\n" << endl;
}

// ##########################################################################################
// main() routine entry 
// ##########################################################################################
int main( int argc, char** argv )
{
	// --------------------------------------------------------------------------------------
	// Tuning parameters
	// --------------------------------------------------------------------------------------
	float ratio = 0.70f;
	double confidence = 0.99;
	double dist = 1.0;
	double keypoints = 10;

	// --------------------------------------------------------------------------------------
	// Argument check 
	// Application takes one argument of path to directory containing images.
	// --------------------------------------------------------------------------------------
	if( argc!=2 )
    {
        readme();
        return 1;
    }
    
	// --------------------------------------------------------------------------------------
	// Log activity to a file
	// --------------------------------------------------------------------------------------
	ofstream fout("matchReport.txt");
	std::time_t result = std::time(NULL);
	fout << "Frog Image Match Report" << endl;		// Log file output
	fout << "Ratio, confidence, distance, keypoints: " << ratio << "\t" << confidence << "\t" << dist << "\t" << keypoints << endl;
	fout << "Start " << std::asctime(std::localtime(&result));		// Log file output

	string path = argv[1];
	string searchData = "\\Rs10*.jpg";
	string fullSearchPath = path + searchData;
	WIN32_FIND_DATA FindData;
	HANDLE hFind = FindFirstFile( (LPCSTR)fullSearchPath.c_str(), &FindData );
	int matchCount = 0;
	fout << "Image directory: " + path << endl;		// Log file output
	if (hFind == INVALID_HANDLE_VALUE)
	{
		cout << "Path is " << path;
		cout << "\nfullSearchPath is " << fullSearchPath << "\n";
		cout << "Error(" << errno << ") opening " << path << endl;
		return errno;
	}

	// --------------------------------------------------------------------------------------
	// Iterate through the images
	// --------------------------------------------------------------------------------------
	do
	{
		string myFileName = FindData.cFileName;
		string orgFile = myFileName;
		string filePath = path + "\\" + myFileName;

		GpuMat imageGpu1, imageGpu2;
		//Mat image1 = imread( filePath, 0 );
		imageGpu1.upload(imread(filePath, CV_LOAD_IMAGE_GRAYSCALE));
		if( imageGpu1.empty() )
		{
			cout << "\n Error, couldn't read image filename " << filePath << endl;
			return 1;
		}
        
		// --------------------------------------------------------------------------------------
		// Loop through directory to test files against the outer loop one by one
		// --------------------------------------------------------------------------------------
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
			imageGpu2.upload(imread(filePath2, CV_LOAD_IMAGE_GRAYSCALE));
			//cv::Mat image2= cv::imread( filePath2, 0 );
		
			if (!imageGpu1.data || !imageGpu2.data)
					return 0; 
			// ---------------------------------------------------------------------------------
			// Prepare the matcher
			// ---------------------------------------------------------------------------------
			FrogMatcher fmatcher;
			fmatcher.setConfidenceLevel(confidence);
			fmatcher.setMinDistanceToEpipolar(dist);
			fmatcher.setRatio(ratio);
			cv::Ptr<cv::FeatureDetector> pfd= new cv::SurfFeatureDetector(keypoints); 
			cv::Ptr<cv::DescriptorExtractor> extractor= new cv::SurfDescriptorExtractor;
			fmatcher.setFeatureDetector(pfd);
			fmatcher.setDescriptorExtractor(extractor);

			// ---------------------------------------------------------------------------------
			// Match the two images
			// ---------------------------------------------------------------------------------
            std::cout << myFileName << " : " << myFileName2 << std::endl;
			std::vector<cv::DMatch> matches;
			std::vector<cv::KeyPoint> keypoints1, keypoints2;
			//cv::Mat fundamental= fmatcher.match(image1,image2,matches, keypoints1, keypoints2);
			Mat fundamental= fmatcher.match(imageGpu1,imageGpu2,matches, keypoints1, keypoints2);

			if (matches.size()>0) {

				// We have a match;  draw the matches
				cv::Mat imageMatches;
				Mat image1, image2;
				image1 = Mat(imageGpu1);
				image2 = Mat(imageGpu2);

				/* -----------------------------------------------------------------------------
				cv::drawMatches(image1,keypoints1,  // 1st image and its keypoints
									image2,keypoints2,  // 2nd image and its keypoints
												matches,                        // the matches
												imageMatches,           // the image produced
												cv::Scalar(255,255,255)); // color of the lines
				------------------------------------------------------------------------------- */

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
        
				// ---------------------------------------------------------------------------------
				// Draw the epipolar lines
				// ---------------------------------------------------------------------------------
				std::vector<cv::Vec3f> lines1; 
				cv::computeCorrespondEpilines(cv::Mat(points1),1,fundamental,lines1);
                
				for (std::vector<cv::Vec3f>::const_iterator it= lines1.begin();
								 it!=lines1.end(); ++it) {

								 cv::line(image2,cv::Point(0,-(*it)[2]/(*it)[1]),
													 cv::Point(image2.cols,-((*it)[2]+(*it)[0]*image2.cols)/(*it)[1]),
																 cv::Scalar(255,255,255));
				}

				std::vector<cv::Vec3f> lines2; 
				cv::computeCorrespondEpilines(cv::Mat(points2),2,fundamental,lines2);
        
				for (std::vector<cv::Vec3f>::const_iterator it= lines2.begin();
							 it!=lines2.end(); ++it) {

								 cv::line(image1,cv::Point(0,-(*it)[2]/(*it)[1]),
													 cv::Point(image1.cols,-((*it)[2]+(*it)[0]*image1.cols)/(*it)[1]),
																 cv::Scalar(255,255,255));
				}

				Mat combinedImage = cvCreateMat(MAX(image1.rows,image2.rows), image1.cols + image2.cols, image1.type());
				cv::Mat tmp = combinedImage(cv::Rect(0, 0, image1.cols, image1.rows));
				image1.copyTo(tmp);
				tmp = combinedImage(cv::Rect(image1.cols, 0, image2.cols, image2.rows));
				image2.copyTo(tmp);

				// ---------------------------------------------------------------------------------
				// Output the combined image (epipolar lines matches)
				// ---------------------------------------------------------------------------------
				string outputFile = ".\\Output\\" + myFileName + ".matches." + myFileName2;
				//string outimFile = ".\\Output\\" + myFileName + ".im." + myFileName2;
				cv::imwrite(outputFile, combinedImage);
				//cv::imwrite(outimFile, imageMatches);
				
				// Write out filenames and 1 to indicate a match
				fout << myFileName + "\t " + myFileName2 + "\t 1" << endl;
			}
			else
				// Write out filenames and 0 to indicate not a match
				fout << myFileName + "\t " + myFileName2 + "\t 0" << endl;
				matchCount += 1;
			}
		}
		// Next iteration (same left image, next right image)
		while( FindNextFile(hFind2, &FindData2 ) >0 );

	}

	// Next iteration  (next left image, restart right images loop)
	while( FindNextFile(hFind, &FindData ) >0 );

	// Exit application
	result = std::time(NULL);
	fout << "Total Matches\t" << matchCount << endl;
	fout << "End " << std::asctime(std::localtime(&result));		// Log file output
	fout.close();
	return 0;
}
