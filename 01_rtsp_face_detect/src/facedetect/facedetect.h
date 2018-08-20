/*
 * facedetect.h
 *
 *  Created on: Aug 18, 2018
 *      Author: cui
 */

#ifndef SRC_FACEDETECT_FACEDETECT_H_
#define SRC_FACEDETECT_FACEDETECT_H_

#include <opencv2/opencv.hpp>
#include "arcsoft_fsdk_face_detection.h"

#define FD_WORKBUF_SIZE        (40*1024*1024)
#define MAX_FACE_NUM        (50)

enum linux_support_fmt{
	I420 = ASVL_PAF_I420,
	NV12 = ASVL_PAF_NV12,
	NV21 = ASVL_PAF_NV21,
	YUYV = ASVL_PAF_YUYV,
	RGB24_B8G8R8 = ASVL_PAF_RGB24_B8G8R8
};


class facedetect{
private:
	MByte *pWorkMem;
	MHandle hEngine;
	ASVLOFFSCREEN inputImg;
	LPAFD_FSDK_FACERES faceResult;

	//用户期望引擎最多能检测出的人脸数 有效值范围[1,50]
	int max_face_number;

//	用于数值表示的最小人脸尺寸 有效值范围[2,50] 推荐值 16。
//	该尺寸是人脸相对于所在图片的长边的占比。例如,如果用户想检测到的最小
//	人脸尺寸是图片长度的 1/8,那么这个 nScale 就应该设置为 8
	int nscale;


public:
	facedetect();

	bool facedetect_init();
	void facedetect_getversion();
	bool  fd_get_inputimage( int img_width, int img_height, cv::Mat &frame );
	bool fd_detect( cv::Mat &frame );

	bool facedetect_uninit();

};


#endif /* SRC_FACEDETECT_FACEDETECT_H_ */
