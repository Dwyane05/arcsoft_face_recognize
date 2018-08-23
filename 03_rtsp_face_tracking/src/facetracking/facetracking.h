/*
 * facetracking.h
 *
 *  Created on: Aug 22, 2018
 *      Author: cui
 */

#ifndef _FACETRACKING_H_
#define _FACETRACKING_H_


#include <opencv2/opencv.hpp>
#include "arcsoft_fsdk_face_tracking.h"

#define FT_WORKBUF_SIZE        (40*1024*1024)
//#define MAX_FACE_NUM        (50)

class facetracking{
private:
	MByte*	FT_pWorkMem;
	MHandle FT_hEngine;
	ASVLOFFSCREEN inputImg;
	LPAFT_FSDK_FACERES faceResult;

	//用户期望引擎最多能检测出的人脸数 有效值范围[1,50]
	int max_face_number;

//	用于数值表示的最小人脸尺寸 有效值范围[2,50] 推荐值 16。
//	该尺寸是人脸相对于所在图片的长边的占比。例如,如果用户想检测到的最小
//	人脸尺寸是图片长度的 1/8,那么这个 nScale 就应该设置为 8
	int nscale;


public:
	facetracking();
	~facetracking();

	bool ft_init();
	void ft_getversion();
	bool  ft_get_inputimage( int img_width, int img_height, cv::Mat &frame );
	bool ft_detect( cv::Mat &frame );

	bool ft_uninit();

};




#endif /* 03_RTSP_FACE_TRACKING_SRC_FACETRACKING_FACETRACKING_H_ */
