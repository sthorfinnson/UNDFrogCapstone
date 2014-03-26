#include <stdio.h>
#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/opencv.hpp"
#include </usr/local/correct/include/mysql.h>


using namespace cv;

void readme();
int save_Descriptors(Mat, string, string, string);
string get_Descriptors(string);
//static char *descriptor_loaction;

/** @function main */
int main( int argc, char** argv )
{
    if( argc != 3 )
    { return -1; }
    
    //FileStorage fs("/Users/sean/Desktop/test.yml", FileStorage::WRITE);
    Mat img_1 = imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE );
    Mat img_2 = imread( argv[2], CV_LOAD_IMAGE_GRAYSCALE );
    
    
    
    
    
    if( !img_1.data || !img_2.data )
    { return -1; }
    
    //-- Step 1: Detect the keypoints using SURF Detector
    int minHessian = 400;
    
    SurfFeatureDetector detector( minHessian );
    
    //brief = cv2.DescriptorExtractor_create("BRIEF");
    
    std::vector<KeyPoint> keypoints_1, keypoints_2;
    
   
    
    detector.detect( img_1, keypoints_1 );
    detector.detect( img_2, keypoints_2 );
    SurfDescriptorExtractor extractor;
    
    Mat descriptors_1, descriptors_2, descriptor_loaded1, Descriptor_loaded2;
    std::vector<KeyPoint> keypoint_loaded1, keypoint_loaded2;
    
    /*
    fs8.open("/Users/sean/Desktop/keypoints8.yml", FileStorage::READ);
    if(fs8.isOpened())
    {
        fs8["descriptors_1"] >> descriptor_loaded1;
        std::cout<<"READING 1\n\n";
        //std::cout<<descriptor_loaded1;
        
    }
    fs8.release();
    fs9.open("/Users/sean/Desktop/keypoints9.yml", FileStorage::READ);
    if(fs9.isOpened())
    {
        fs9["descriptors_2"] >> descriptor_loaded2;
         std::cout<<"READING 2\n\n";
        //std::cout<<descriptor_loaded2;
    }
    fs9.release();
     */
     
    
    
    extractor.compute( img_1, keypoints_1, descriptors_1 );
    if (save_Descriptors(descriptors_1, "/Users/sean/Desktop/keypoints10.yml", "Rs10-141.jpg", "/Users/sean/Downloads/Rs2010/Rs10-141.jpg")==0){
        std::cout<<"success!!"<<std::endl;
    }
    
    
    string location;
    location = get_Descriptors("Rs10-141.jpg");
    FileStorage fs(location, FileStorage::READ);
    if(fs.isOpened()){
        fs["descriptors"] >> descriptor_loaded1;
    }
    
    extractor.compute( img_2, keypoints_2, descriptors_2 );
   
    
    
    
    
    
    //-- Step 3: Matching descriptor vectors with a brute force matcher
    BFMatcher matcher(NORM_L2);
    std::vector< DMatch > matches;
   // matcher.match( descriptors_1, descriptors_2, matches );
    matcher.match( descriptor_loaded1, descriptors_2, matches );
    //std::cout<<descriptor_loaded2;
    
    if(matches.size() > 0){
        std::cout<<"we have a match";
    }
    
    //-- Draw matches
    Mat img_matches;
    drawMatches( img_1, keypoints_1, img_2, keypoints_2, matches, img_matches );
    
    //-- Show detected matches
    imshow("Matches", img_matches );
    
    waitKey(0);
    
    return 0;
}

/** @function readme */
void readme()
{ std::cout << " Usage: ./SURF_descriptor <img1> <img2>" << std::endl; }

int save_Descriptors(Mat descriptors_In, string path, string imgName, string img_Directory){
   
	MYSQL *connection, mysql;
	
	int state;
	
	mysql_init(&mysql);
	
	connection = mysql_real_connect(&mysql,"localhost","root","root","woodfrogs",0,0,0);
	if (connection == NULL)
	{
        std::cout<<"FAILED HERE 1"<<std::endl;
		std::cout << mysql_error(&mysql) << std::endl;
		
		return 1;
	}
    FileStorage fs(path+imgName+".yml", FileStorage::WRITE);
    write(fs, "descriptors", descriptors_In);
    fs.release();
    
    std::string prepared_Statement = "INSERT INTO descriptors(descriptor_location, image_id, image_location) VALUES('"+path+"', '"+imgName+"', '"+img_Directory+"')";
    state = mysql_query(connection, prepared_Statement.c_str());
    
    if(state !=0){
        std::cout<<"FAILED HERE 2"<<std::endl;
        std::cout<<mysql_error(connection)<<std::endl<<std::endl;
        return 1;
    }
    mysql_close(connection);
    return 0;
    
}

string get_Descriptors(string imgName){
    
	MYSQL *connection, mysql;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int state;
	
	mysql_init(&mysql);
	
	connection = mysql_real_connect(&mysql,"localhost","root","root","woodfrogs",0,0,0);
	if (connection == NULL)
	{
		std::cout << mysql_error(&mysql) << std::endl;
		return 0;
		
	}
    
    
    std::string prepared_Statement = "SELECT * FROM descriptors WHERE image_id = '"+imgName+"'";
    state = mysql_query(connection, prepared_Statement.c_str());
    
    if(state !=0){
        std::cout<<mysql_error(connection)<<std::endl<<std::endl;
        return 0;
    }
    result = mysql_store_result(connection);
    row = mysql_fetch_row(result);
    return row[0];
    mysql_close(connection);
    return 0;
    
}
