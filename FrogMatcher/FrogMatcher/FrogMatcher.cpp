// FrogMatcher.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"					// For Visual Studio
#include "FrogMatcher.h"
#include <iostream>
#include <vector>
#include <opencv2/calib3d/calib3d.hpp>
#include <fstream>
#include <string>
#include <cstdlib>
#include <windows.h>				// For compiling in Windows
#include <ctime>
#include <dirent.h>


using namespace std;
using namespace cv;
using namespace cv::gpu;

// --------------------------------------------------------------------------------------
// Globals
// --------------------------------------------------------------------------------------
std::vector<std::string> candidateImage, dbImage;		// holds file search results
std::string imageType = "jpg";							// jpg, jpeg, png
std::string candidateSet, dbSet;	// Designation for current set and db to search


// ##########################################################################################
// Readme instructions for command-line 
// ##########################################################################################
static void readme()
{
    cout << "\nThis enhanced image matching application was developed to match frogs from\n"
            "a frog identification database.  \n"
			"Set1 and Set2 are 4-character designations and should be the first 4 chars\n"
			"of each set of images.  Example:  Rs10 for images Rs10*.jpg\n"
            "Design details to be added.\n"
            "Usage:\n"
            "./FrogMatcher.exe <path_to_folder> <Set1> <Set2>\n" << endl;
}

// ##########################################################################################
// File search routine to locate image sets 
// ##########################################################################################
void search(std::string curr_directory, std::string extension){
	DIR* dir_point = opendir(curr_directory.c_str());
	dirent* entry = readdir(dir_point);
	while (entry){									// if !entry then end of directory
		/* ---------------------------------------------------------------------------------
		if (entry->d_type == DT_DIR){				// if entry is a directory
			std::string fname = entry->d_name;
			if (fname != "." && fname != "..")
				search(entry->d_name, extension);	// search through it
		}
		else 
		-------------------------------------------------- */
		if (entry->d_type == DT_REG){		// if entry is a regular file
			std::string fname = entry->d_name;	// filename
					//std::cout << "Found " << fname << std::endl;

												// if filename's last characters are extension
			if (fname.find(extension, (fname.length() - extension.length())) != std::string::npos){
				if (fname.compare(0,4,candidateSet) == 0){
					candidateImage.push_back(fname);
				}
				else
				{
					if (fname.compare(0,4,dbSet) == 0)
						dbImage.push_back(fname);		// add filename to results vector
				}
			}
		}
		entry = readdir(dir_point);
	}
	return;
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
	if( argc!=4 )
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

	// --------------------------------------------------------------------------------------
	// Setup image search parameters
	// --------------------------------------------------------------------------------------
	std::string curr_directory = argv[1];
	candidateSet = argv[2];					// Example:  Rs10 for 2010 set
	dbSet = argv[3];						// Example:  Rs09 for 2009 set
	int matchCount = 0;

	// --------------------------------------------------------------------------------------
	// Perform search for images in target directory
	// --------------------------------------------------------------------------------------
	search(curr_directory, imageType);


	// --------------------------------------------------------------------------------------
	// Iterate through the images
	// --------------------------------------------------------------------------------------
	if (candidateImage.size() && dbImage.size()){
		std::clog << candidateImage.size() << " candidate images were found:" << std::endl;

		// Outer loop for test candidate images.
		for (int i = 0; i < candidateImage.size(); ++i)	
			{
				std::cout << candidateImage[i] << std::endl;
				GpuMat imageGpu1, imageGpu2;
				string filePath = curr_directory + "//" + candidateImage[i];
				imageGpu1.upload(imread(filePath, CV_LOAD_IMAGE_GRAYSCALE));
				if( imageGpu1.empty() )
				{
					cout << "\n Error, couldn't read image filename " << filePath << endl;
					return 1;
				}

				// Inner loop for target db images
				for (unsigned int j = 0; j < dbImage.size(); ++j)	// used unsigned to appease compiler warnings
				{
					std::cout << "- \t" <<  dbImage[j] << std::endl;
					string filePath2 = curr_directory + "//" + dbImage[j];
					imageGpu2.upload(imread(filePath2, CV_LOAD_IMAGE_GRAYSCALE));
					if (!imageGpu1.data || !imageGpu2.data)		// Check for empty image matrix
						return 0; 
					// ---------------------------------------------------------------------------------
					// Prepare the matcher
					// ---------------------------------------------------------------------------------
					FrogMatcher fmatcher;
					fmatcher.setConfidenceLevel(confidence);
					fmatcher.setMinDistanceToEpipolar(dist);
					fmatcher.setRatio(ratio);
					//cv::Ptr<cv::FeatureDetector> pfd= new cv::SurfFeatureDetector(keypoints); 
					//cv::Ptr<cv::DescriptorExtractor> extractor= new cv::SurfDescriptorExtractor;
					//fmatcher.setFeatureDetector(pfd);
					//fmatcher.setDescriptorExtractor(extractor);

					// ---------------------------------------------------------------------------------
					// Match the two images
					// ---------------------------------------------------------------------------------
					std::cout << candidateImage[i] << " : " << dbImage[j] << std::endl;
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
						string outputFile = ".//Output//" + candidateImage[i] + ".matches." + dbImage[j];
						//string outimFile = ".\\Output\\" + myFileName + ".im." + myFileName2;
						cv::imwrite(outputFile, combinedImage);
						//cv::imwrite(outimFile, imageMatches);
				
						// Write out filenames and 1 to indicate a match
						fout << candidateImage[i] + "\t " + dbImage[j] + "\t 1" << endl;
					}
					else{
						// Write out filenames and 0 to indicate not a match
						fout << candidateImage[i] + "\t " + dbImage[j] + "\t 0" << endl;
						matchCount += 1;
					}
				}
		}
	}
	else{
		std::cout << "No files ending in '" << imageType << "' were found." << std::endl;
	}
	

	// Exit application
	result = std::time(NULL);
	fout << "Total Matches\t" << matchCount << endl;
	fout << "End " << std::asctime(std::localtime(&result));		// Log file output
	fout.close();
	return 0;
}
