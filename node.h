/*
 * node.h
 *
 *  Created on: Nov 22, 2014
 *      Author: aylin
 */

#ifndef SRC_NODE_H_
#define SRC_NODE_H_

#include <iostream>
#include "ros/ros.h"
#include "std_msgs/String.h"
#include <pcl_ros/point_cloud.h>

#include "cv.h"
#include "highgui.h"
#include "opencv2/imgproc/imgproc.hpp"

//for publishing and subscribing to images
//in ROS allows you to subscribe to compressed image streams
#include <image_transport/image_transport.h>

//includes the header for CvBridge as well as some useful constants and functions related to image encodings
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>

//Includes the headers for OpenCV's image processing and GUI modules
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <opencv2/video/background_segm.hpp>

//Timer
#include <cstdio>
#include <ctime>

using namespace ros;
using namespace sensor_msgs;
using namespace std;
using namespace cv;
namespace enc = sensor_msgs::image_encodings;
#include"waterfilling.h"

class Subscribe_Depth {
	cv_bridge::CvImagePtr cv_ptr;
	NodeHandle nh;
	Subscriber sub;
	Mat a, b;
	double min, max;
	Ptr<BackgroundSubtractor> pMOG1;
	Mat fgMaskMOG1;

public:
	Subscribe_Depth() {

		sub = nh.subscribe("/camera/depth/image", 1, &Subscribe_Depth::callback,
				this);
		pMOG1 = new BackgroundSubtractorMOG(); //MOG2 approach

	}
	Mat depthSubtrac(Mat depth_map, Mat back) {
		typedef uchar imgType;
		imgType* m_a = NULL;
		imgType* m_b = NULL;
		imgType* m_c = NULL;

		int picWidth = depth_map.cols; //picture width
		int picHeight = depth_map.rows; //picture height

		Mat outputImg(picHeight, picWidth, depth_map.type()); //initialize the output image

		m_a = (imgType*) back.data;
		m_b = (imgType*) depth_map.data;
		m_c = (imgType*) outputImg.data;

		for (int i = 0; i < picHeight * picWidth; i++, m_a++, m_b++, m_c++) {
			if (*m_a < 2) {
				*m_a = 0;
				*m_c = (*m_a) * (*m_b);

			} else {
				*m_a = 1;
				*m_c = (*m_a) * (*m_b);

			}

		}

		return outputImg;
	}

	~Subscribe_Depth() {
	}

	void callback(const sensor_msgs::ImageConstPtr& im) {
		clock_t start1, end1, start2,end2,start3,end3,start4,end4;
		double duration1,duration2,duration3,duration4;



		Mat depth_map, sub_map, filter_map, new_im, temp;

		//convert ros to opencv (32 bits float)
		cv_ptr = cv_bridge::toCvCopy(im,sensor_msgs::image_encodings::TYPE_32FC1);

		//min max range
		minMaxIdx(cv_ptr->image, &min, &max);

		// expand your range to 0..255. Similar to histEq();
		/*
		 * CV_8U is unsigned 8bit/pixel - ie a pixel can have values 0-255,
		 * this is the normal range for most image and video formats.
		 *
		 */
		cv_ptr->image.convertTo(depth_map, CV_8U, 255 / (max - min), -min);

		Mat median_depth;

		medianBlur(depth_map,median_depth , 19);

		pMOG1->operator()(median_depth, fgMaskMOG1);

		Mat outputImg = depthSubtrac(median_depth, fgMaskMOG1);


		WaterFilling water_obje;

		Mat result = water_obje.WaterDrop(outputImg, 1000);



		Mat thresh_im,median_depth2;
		threshold(result,thresh_im,120,255,THRESH_BINARY);

		medianBlur(thresh_im,median_depth2 , 19);

		imshow("depth",depth_map);
		imshow("median depth", median_depth);
		imshow("thresh", thresh_im);
		imshow("median_depth2", median_depth2);
		imshow("out",outputImg);
		imshow("waterfilling", result);
		imwrite("/home/aylin/Desktop/depth.png", depth_map);
		imwrite("/home/aylin/Desktop/out.png", outputImg);
		imwrite("/home/aylin/Desktop/water.png", result);
		imwrite("/home/aylin/Desktop/thresh.png", thresh_im);
		imwrite("/home/aylin/Desktop/median_depth2.png", median_depth2);
		waitKey(3);



	}

};

#endif /* SRC_NODE_H_ */
