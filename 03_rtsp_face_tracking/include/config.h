/*
 * config.h
 *
 *  Created on: Aug 18, 2018
 *      Author: cui
 */

#ifndef INCLUDE_CONFIG_H_
#define INCLUDE_CONFIG_H_

enum linux_support_fmt{
	I420 = ASVL_PAF_I420,
	NV12 = ASVL_PAF_NV12,
	NV21 = ASVL_PAF_NV21,
	YUYV = ASVL_PAF_YUYV,
	RGB24_B8G8R8 = ASVL_PAF_RGB24_B8G8R8
};

#define	PIXEL_FORMAT	RGB24_B8G8R8
/*******************************************
 * 测试使用数据格式：RGB24_B8G8R8	时，人脸匹配度低一点(0.674384)
				使用I420格式时，匹配度高一点(0.707299)		rtsp原始码流格式
				NV12:待验证
        		NV21:待验证
        		YUYV:待验证  属于422，应该匹配度会更高

        		I420: YYYYYYYY UU VV    =>YUV420P
				YV12: YYYYYYYY VV UU    =>YUV420P
				NV12: YYYYYYYY UVUV     =>YUV420SP
				NV21: YYYYYYYY VUVU     =>YUV420SP
 *******************************************/


#define APPID     		"EokKppfrCynPrtaSsxFUFDWYurD7zhyxtCMzAfnsFq11"
#define FD_SDKKEY  		"7Ybt3PXgYniBxEuDg6etvEJGpard3XEHVgPVxfLHVNgd"
#define FR_SDKKEY		"7Ybt3PXgYniBxEuDg6etvEJmUBuKrqhTi2twiSiorj37"
#define	FT_SDKKEY		"7Ybt3PXgYniBxEuDg6etvEJ9fBbPtUwEGG5JnmKJQTL4"

//APP_ID:				EokKppfrCynPrtaSsxFUFDWYurD7zhyxtCMzAfnsFq11
//人脸追踪(FT):			7Ybt3PXgYniBxEuDg6etvEJ9fBbPtUwEGG5JnmKJQTL4
//人脸检测(FD):			7Ybt3PXgYniBxEuDg6etvEJGpard3XEHVgPVxfLHVNgd
//人脸识别(FR):			7Ybt3PXgYniBxEuDg6etvEJmUBuKrqhTi2twiSiorj37
//年龄识别(Age):		7Ybt3PXgYniBxEuDg6etvEK1nzReu53XWyvGtCBBqZXM
//性别识别(Gender):		7Ybt3PXgYniBxEuDg6etvEK8xPgqVs7b6qrKCMzJhYoC


//	rtsp://admin:ijx967111@192.168.5.11/h264/ch1/main/av_stream
//	rtsp://admin:ijx967111@192.168.5.11/h264/ch1/sub/av_stream
#endif /* INCLUDE_CONFIG_H_ */
