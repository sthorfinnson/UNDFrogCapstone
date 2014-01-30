#pragma once
#if !defined MATCHER
#define MATCHER

#include <vector>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>		//needed for SurfFeatureDetector
#include <opencv2/calib3d/calib3d.hpp>			//needed for  RANSAC CV_FM_8POINT 
#include "opencv2/gpu/gpu.hpp"
#include "opencv2/nonfree/gpu.hpp"

class FrogMatcher {

  private:

          // pointer to the feature point detector object
          cv::Ptr<cv::FeatureDetector> detector;
          // pointer to the feature descriptor extractor object
          cv::Ptr<cv::DescriptorExtractor> extractor;
		  cv::Ptr<cv::DescriptorMatcher> matcher;
          float ratio; // max ratio between 1st and 2nd NN
          bool refineF; // if true will refine the F matrix
          double distance; // min distance to epipolar
          double confidence; // confidence level (probability)
		  int matchedPoints1; // number of matched points 1->2
		  int matchedPoints2; // number of matched points 2->1
		  int ratioTest1; // number of matched points following ratio test 1->2
		  int ratioTest2; // number of matched points following ratio test 2->1
		  int symTest; // number of matched points following symmetry test
		  int ransacResults; // number of matched points following RANSAC

  public:

          FrogMatcher() : ratio(0.85f), refineF(true), confidence(0.99), distance(3.0) {        

                  // SURF is the default feature detector
                  detector= new cv::SurfFeatureDetector();
                  extractor= new cv::SurfDescriptorExtractor();
                  //detector= new cv::OrbFeatureDetector();
                  //extractor= new cv::OrbDescriptorExtractor();
				  // FLANN is the default descriptor matcher
				  //matcher = new cv::FlannBasedMatcher;
                  //cv::BFMatcher matcher;
		          //matcher= new cv::BFMatcher;

          }

		// ------------------------------------------------------------------------------
		// Get properties
		// ------------------------------------------------------------------------------
		int getMatchedPoints1() {
			return matchedPoints1;
		}

		int getMatchedPoints2() {
			return matchedPoints2;
		}

		int getRatioTest1() {
			return ratioTest1;
		}

		int getRatioTest2() {
			return ratioTest2;
		}

		int getSymTest() {
			return symTest;
		}

		int getRansac() {
			return ransacResults;
		}

         // Set the feature detector
          void setFeatureDetector(cv::Ptr<cv::FeatureDetector>& detect) {

                  detector= detect;
          }

          // Set descriptor extractor
          void setDescriptorExtractor(cv::Ptr<cv::DescriptorExtractor>& desc) {

                  extractor= desc;
          }

		  // Set the matcher
		  //void setDescriptorMatcher(cv::Ptr<cv::DescriptorMatcher>& match) {
     
			//  matcher= match;

		  //}

          // Set the minimum distance to epipolar in RANSAC
          void setMinDistanceToEpipolar(double d) {

                  distance= d;
          }

          // Set confidence level in RANSAC
          void setConfidenceLevel(double c) {

                  confidence= c;
          }

          // Set the NN ratio
          void setRatio(float r) {

                  ratio= r;
          }

          // if you want the F matrix to be recalculated
          void refineFundamental(bool flag) {

                  refineF= flag;
          }


		// ##########################################################################################
		// ratioTest Function
        // Clear matches for which NN ratio is > than threshold
		// return the number of removed points
 		// ##########################################################################################
          int ratioTest(std::vector<std::vector<cv::DMatch>>& matches) {

                int removed=0;

        // for all matches
                for (std::vector<std::vector<cv::DMatch>>::iterator matchIterator= matches.begin();
                         matchIterator!= matches.end(); ++matchIterator) {

                                 // if 2 NN has been identified
                                 if (matchIterator->size() > 1) {

                                         // check distance ratio
                                         if ((*matchIterator)[0].distance/(*matchIterator)[1].distance > ratio) {

                                                 matchIterator->clear(); // remove match
                                                 removed++;
                                         }

                                 } else { // does not have 2 neighbours

                                         matchIterator->clear(); // remove match
                                         removed++;
                                 }
                }

                return removed;
          }

		// ##########################################################################################
		// symmetryTest Function
        // Insert symmetrical matches in symMatches vector
 		// ##########################################################################################
          void symmetryTest(const std::vector<std::vector<cv::DMatch>>& matches1,
                                const std::vector<std::vector<cv::DMatch>>& matches2,
                                            std::vector<cv::DMatch>& symMatches) {
                        
                // for all matches image 1 -> image 2
                for (std::vector<std::vector<cv::DMatch>>::const_iterator matchIterator1= matches1.begin();
                         matchIterator1!= matches1.end(); ++matchIterator1) {

                        if (matchIterator1->size() < 2) // ignore deleted matches 
                                continue;

                        // for all matches image 2 -> image 1
                        for (std::vector<std::vector<cv::DMatch>>::const_iterator matchIterator2= matches2.begin();
                                matchIterator2!= matches2.end(); ++matchIterator2) {

                                if (matchIterator2->size() < 2) // ignore deleted matches 
                                        continue;

                                // Match symmetry test
                                if ((*matchIterator1)[0].queryIdx == (*matchIterator2)[0].trainIdx  && 
                                        (*matchIterator2)[0].queryIdx == (*matchIterator1)[0].trainIdx) {

                                                // add symmetrical match
                                                symMatches.push_back(cv::DMatch((*matchIterator1)[0].queryIdx,
                                                                                                            (*matchIterator1)[0].trainIdx,
                                                                                                            (*matchIterator1)[0].distance));
                                                break; // next match in image 1 -> image 2
                                }
                        }
                }
          }

		// ##########################################################################################
		// RANSACTEST Function
        // Identify good matches using RANSAC
        // Return fundemental matrix
		// ##########################################################################################
          cv::Mat ransacTest(const std::vector<cv::DMatch>& matches,
                                 const std::vector<cv::KeyPoint>& keypoints1, 
                                                 const std::vector<cv::KeyPoint>& keypoints2,
                                             std::vector<cv::DMatch>& outMatches) {

				//cv::gpu::GpuMat fundamentalGPU;
                // Convert keypoints into Point2f       
                std::vector<cv::Point2f> points1, points2;      
                for (std::vector<cv::DMatch>::const_iterator it= matches.begin();
                         it!= matches.end(); ++it) {

                         // Get the position of left keypoints
                         float x= keypoints1[it->queryIdx].pt.x;
                         float y= keypoints1[it->queryIdx].pt.y;
                         points1.push_back(cv::Point2f(x,y));
                         // Get the position of right keypoints
                         x= keypoints2[it->trainIdx].pt.x;
                         y= keypoints2[it->trainIdx].pt.y;
                         points2.push_back(cv::Point2f(x,y));
            }

                // Compute F matrix using RANSAC
                std::vector<uchar> inliers(points1.size(),0);
                cv::Mat fundamental= cv::findFundamentalMat(
                        cv::Mat(points1),cv::Mat(points2), // matching points
                    inliers,      // match status (inlier ou outlier)  
                    CV_FM_RANSAC, // RANSAC method
                    distance,     // distance to epipolar line
                    confidence);  // confidence probability
        
                // extract the surviving (inliers) matches
                std::vector<uchar>::const_iterator itIn= inliers.begin();
                std::vector<cv::DMatch>::const_iterator itM= matches.begin();
                // for all matches
                for ( ;itIn!= inliers.end(); ++itIn, ++itM) {

                        if (*itIn) { // it is a valid match

                                outMatches.push_back(*itM);
                        }
                }

                std::cout << "Number of matched points (after RANSAC): " << outMatches.size() << std::endl;

				if (outMatches.size()>0) {

					if (refineF) {
					// The F matrix will be recomputed with all accepted matches

							// Convert keypoints into Point2f for final F computation       
							points1.clear();
							points2.clear();
        
							for (std::vector<cv::DMatch>::const_iterator it= outMatches.begin();
									 it!= outMatches.end(); ++it) {

									 // Get the position of left keypoints
									 float x= keypoints1[it->queryIdx].pt.x;
									 float y= keypoints1[it->queryIdx].pt.y;
									 points1.push_back(cv::Point2f(x,y));
									 // Get the position of right keypoints
									 x= keypoints2[it->trainIdx].pt.x;
									 y= keypoints2[it->trainIdx].pt.y;
									 points2.push_back(cv::Point2f(x,y));
							}

							// Compute 8-point F from all accepted matches
							fundamental= cv::findFundamentalMat(
									cv::Mat(points1),cv::Mat(points2), // matching points
									CV_FM_8POINT); // 8-point method
					}
					//fundamentalGPU.upload(fundamental);
					return fundamental;
				}
				else
	                std::cout << "Frogs are not a match!!" << std::endl;
					
							// Compute 8-point F from all accepted matches
							//cv::Mat(points2) = cv::Mat(points1);
							//fundemental= cv::findFundamentalMat(
							//		cv::Mat(points1),cv::Mat(points2), // matching points
							//		CV_FM_8POINT); // 8-point method
					//fundamentalGPU.upload(fundamental);
					return fundamental;
          }

		// ##########################################################################################
		// match Function
        // Match feature points using symmetry test and RANSAC
        // returns fundemental matrix
		// ##########################################################################################
          cv::Mat match(cv::gpu::GpuMat& image1, cv::gpu::GpuMat& image2, // input images 
                  std::vector<cv::DMatch>& matches, // output matches and keypoints
                  std::vector<cv::KeyPoint>& keypoints1, std::vector<cv::KeyPoint>& keypoints2) {

                // 1. Detection of the keypoint features, compute descriptors
				cv::gpu::GpuMat keypoints1GPU, keypoints2GPU;
				cv::gpu::GpuMat descriptors1GPU, descriptors2GPU;

				// --------------------------------------------------------------------------------------
				// SURF 
				// --------------------------------------------------------------------------------------
				cv::gpu::SURF_GPU surf;
				surf(image1, cv::gpu::GpuMat(), keypoints1GPU, descriptors1GPU);
				surf(image2, cv::gpu::GpuMat(), keypoints2GPU, descriptors2GPU);
 

				// --------------------------------------------------------------------------------------
				// ORB 
				// --------------------------------------------------------------------------------------
				/* 
				cv::gpu::ORB_GPU orb(10000);
				orb.blurForDescriptor = true;
				cv::gpu::GpuMat fullmask(image1.size(), CV_8U, 0xFF);
				orb(image1, fullmask, keypoints1GPU, descriptors1GPU);
                cv::Mat descriptors1, descriptors2;
                std::cout << "ORB_GPU: image1 keypts and descr" << std::endl;
				orb.release();
				fullmask.release();
				fullmask.create(image2.size(), CV_8U);
				fullmask.setTo(0xFF);
				orb(image2, fullmask, keypoints2GPU, descriptors2GPU);
                std::cout << "ORB_GPU: image2 keypts and descr" << std::endl;
				fullmask.release();
				orb.downloadKeyPoints(keypoints1GPU, keypoints1);
				orb.downloadKeyPoints(keypoints2GPU, keypoints2);
				descriptors1GPU.release();
                std::cout << "ORB_GPU: keypts and desc downloaded" << std::endl;
				--------------------------------------------------------------------------------------- */

                // 1b. Extraction of the SURF descriptors
                //cv::Mat descriptors1, descriptors2;
				//descriptors1GPU.download(descriptors1);
				//descriptors2GPU.download(descriptors2);

                // 2. Match the two image descriptors

                // Construction of the matcher 
				cv::gpu::BFMatcher_GPU matcher(4);
				//cv::gpu::GpuMat trainIdx, distance;
				// Initial is BruteForceMatcher: BFMatcher
                //cv::BFMatcher matcher;
                std::cout << "BFMatcher engaged" << std::endl;

                // from image 1 to image 2
                // based on k nearest neighbours (with k=2)
                std::vector<std::vector<cv::DMatch>> matches1;
                matcher.knnMatch(descriptors1GPU,descriptors2GPU, 
                        matches1, // vector of matches (up to 2 per entry) 
                        2);               // return 2 nearest neighbours
                std::cout << "knnMatch 1->2" << std::endl;

                // from image 2 to image 1
                // based on k nearest neighbours (with k=2)
                std::vector<std::vector<cv::DMatch>> matches2;
                matcher.knnMatch(descriptors2GPU,descriptors1GPU, 
                        matches2, // vector of matches (up to 2 per entry) 
                        2);               // return 2 nearest neighbours
                std::cout << "knnMatch 2->1" << std::endl;

				// 2b. Download results
				surf.downloadKeypoints(keypoints1GPU, keypoints1);
				surf.downloadKeypoints(keypoints2GPU, keypoints2);
				//surf.downloadDescriptors(descriptors1GPU, descriptors1);
				//surf.downloadDescriptors(descriptors2GPU, descriptors2);
				//orb.downloadKeyPoints(keypoints1GPU, keypoints1);
				//orb.downloadKeyPoints(keypoints2GPU, keypoints2);
				//descriptors1GPU.download(descriptors1);
				//descriptors2GPU.download(descriptors2);
				//descriptors1GPU.release();
				//BFMatcher_GPU::knnMatchDownload(trainIdx, distance, matches);

                std::cout << "Number of SURF points (1): " << keypoints1.size() << std::endl;
                std::cout << "Number of SURF points (2): " << keypoints2.size() << std::endl;
				std::cout << "Number of matched points 1->2: " << matches1.size() << std::endl;
                std::cout << "Number of matched points 2->1: " << matches2.size() << std::endl;

                // 3. Remove matches for which NN ratio is > than threshold

                // clean image 1 -> image 2 matches
                int removed= ratioTest(matches1);
                std::cout << "Number of matched points 1->2 (ratio test) : " << matches1.size()-removed << std::endl;
                // clean image 2 -> image 1 matches
                removed= ratioTest(matches2);
                std::cout << "Number of matched points 2->1 (ratio test) : " << matches2.size()-removed << std::endl;

                // 4. Remove non-symmetrical matches
				std::vector<cv::DMatch> symMatches;
                symmetryTest(matches1,matches2,symMatches);

                std::cout << "Number of matched points (symmetry test): " << symMatches.size() << std::endl;

				// If there are no symmetrically matched points, skip the RANSAC test (which will fail)
				if (symMatches.size()==0)
					{
						cv::Mat fundamental;
						return fundamental;
				}

				// 5. Validate matches using RANSAC
				cv::Mat fundamental= ransacTest(symMatches, keypoints1, keypoints2, matches);
				
                // return the found fundemental matrix
                return fundamental;
        }
};

#endif



