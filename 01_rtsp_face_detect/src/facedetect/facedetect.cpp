/*
 * facedetect.cpp
 *
 *  Created on: Aug 18, 2018
 *      Author: cui
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <iostream>

#include "facedetect.h"
#include "merror.h"

#include "config.h"
#include "joinus.h"


using namespace cv;

facedetect::facedetect()
{
	pWorkMem = nullptr;
	hEngine = nullptr;
	inputImg = { 0 };

	max_face_number = 50;
	nscale = 16;
}

bool facedetect::facedetect_init()
{
	pWorkMem = (MByte *)malloc(FD_WORKBUF_SIZE);
	if(pWorkMem == nullptr){
		err_quit("face detect init error: fail to malloc workbuf\r\n");
	}

	int ret = AFD_FSDK_InitialFaceEngine(APPID, FD_SDKKEY, pWorkMem, FD_WORKBUF_SIZE,
										 &hEngine, AFD_FSDK_OPF_0_HIGHER_EXT, nscale, max_face_number );
	if (ret != 0) {
		switch(ret){
		case MERR_INVALID_PARAM:
			err_sys( "AFD_FSDK_InitialFaceEngine(): 0x%x  invalid parameter\r\n", ret );
			break;
		case MERR_NO_MEMORY:
			err_sys( "AFD_FSDK_InitialFaceEngine(): 0x%x  no memory\r\n", ret );
			break;
		default:
			err_sys( "fail to AFD_FSDK_InitialFaceEngine(): 0x%x\r\n", ret);
		}
		free(pWorkMem);
		return false;
	}
	DBG( "face detect init success\n" );
	return true;
}

void facedetect::facedetect_getversion()
{
	const AFD_FSDK_Version*pVersionInfo = AFD_FSDK_GetVersion(hEngine);
	printf("Codebace:%d\tMajor:%d\tMinor:%d\tBuild:%d\r\n", pVersionInfo->lCodebase, pVersionInfo->lMajor,
								 pVersionInfo->lMinor, pVersionInfo->lBuild);
	printf("Version:%s\r\n", pVersionInfo->Version);
	printf("Buildtime:%s\r\n", pVersionInfo->BuildDate);
	printf("CopyRight:%s\r\n", pVersionInfo->CopyRight);
}

bool  facedetect::fd_get_inputimage( int img_width, int img_height, cv::Mat &frame )
{
	inputImg.u32PixelArrayFormat = RGB24_B8G8R8;
	inputImg.i32Width = img_width;
	inputImg.i32Height = img_height;
	inputImg.ppu8Plane[0] = (MUInt8*)frame.data;

	if (ASVL_PAF_I420 == inputImg.u32PixelArrayFormat) {
		inputImg.pi32Pitch[0] = inputImg.i32Width;
		inputImg.pi32Pitch[1] = inputImg.i32Width/2;
		inputImg.pi32Pitch[2] = inputImg.i32Width/2;
		inputImg.ppu8Plane[1] = inputImg.ppu8Plane[0] + inputImg.pi32Pitch[0] * inputImg.i32Height;
		inputImg.ppu8Plane[2] = inputImg.ppu8Plane[1] + inputImg.pi32Pitch[1] * inputImg.i32Height/2;
	} else if (ASVL_PAF_NV12 == inputImg.u32PixelArrayFormat) {
		inputImg.pi32Pitch[0] = inputImg.i32Width;
		inputImg.pi32Pitch[1] = inputImg.i32Width;
		inputImg.ppu8Plane[1] = inputImg.ppu8Plane[0] + (inputImg.pi32Pitch[0] * inputImg.i32Height);
	} else if (ASVL_PAF_NV21 == inputImg.u32PixelArrayFormat) {
		inputImg.pi32Pitch[0] = inputImg.i32Width;
		inputImg.pi32Pitch[1] = inputImg.i32Width;
		inputImg.ppu8Plane[1] = inputImg.ppu8Plane[0] + (inputImg.pi32Pitch[0] * inputImg.i32Height);
	} else if (ASVL_PAF_YUYV == inputImg.u32PixelArrayFormat) {
		inputImg.pi32Pitch[0] = inputImg.i32Width*2;
	} else if (ASVL_PAF_I422H == inputImg.u32PixelArrayFormat) {
		inputImg.pi32Pitch[0] = inputImg.i32Width;
		inputImg.pi32Pitch[1] = inputImg.i32Width / 2;
		inputImg.pi32Pitch[2] = inputImg.i32Width / 2;
		inputImg.ppu8Plane[1] = inputImg.ppu8Plane[0] + inputImg.pi32Pitch[0] * inputImg.i32Height;
		inputImg.ppu8Plane[2] = inputImg.ppu8Plane[1] + inputImg.pi32Pitch[1] * inputImg.i32Height;
	} else if (ASVL_PAF_LPI422H == inputImg.u32PixelArrayFormat) {
		inputImg.pi32Pitch[0] = inputImg.i32Width;
		inputImg.pi32Pitch[1] = inputImg.i32Width;
		inputImg.ppu8Plane[1] = inputImg.ppu8Plane[0] + (inputImg.pi32Pitch[0] * inputImg.i32Height);
	} else if (ASVL_PAF_RGB24_B8G8R8 == inputImg.u32PixelArrayFormat) {
		inputImg.pi32Pitch[0] = inputImg.i32Width*3;
	} else {
		err_sys( "face detect error: unsupported Image format: 0x%x\r\n",inputImg.u32PixelArrayFormat );
		return false;
//		free(inputImg.ppu8Plane[0]);
//		AFD_FSDK_UninitialFaceEngine(hEngine);
//		free(pWorkMem);
	}
	return true;
}


bool facedetect::fd_detect( cv::Mat &frame )
{
	//根据输入的图像检测出人脸位置,一般用于静态图像检测
	int ret = AFD_FSDK_StillImageFaceDetection(hEngine, &inputImg, &faceResult);
	if( ret != MOK ) {
		err_sys( "fail to AFD_FSDK_StillImageFaceDetection(): 0x%x\r\n", ret);
		return false;
	}

	for (int i = 0; i < faceResult->nFace; i++) {
		printf("face %d:(%d,%d,%d,%d)\r\n", i,
			   faceResult->rcFace[i].left, faceResult->rcFace[i].top,
			   faceResult->rcFace[i].right, faceResult->rcFace[i].bottom);

		rectangle(frame, Point(faceResult->rcFace[i].left, faceResult->rcFace[i].top),
			Point(faceResult->rcFace[i].right, faceResult->rcFace[i].bottom),
			Scalar(255, 0, 0), 2);
	}
	return true;
}

bool facedetect::facedetect_uninit()
{

	int ret = AFD_FSDK_UninitialFaceEngine(hEngine);
	if( ret != MOK ){
		switch(ret){
		case MERR_INVALID_PARAM:
			err_sys( "AFD_FSDK_UninitialFaceEngine() error--invalid parameter: 0x%x\r\n", ret );
			break;
		default:
			err_sys( "fail to AFD_FSDK_UninitialFaceEngine(): 0x%x\r\n", ret );
		}
	}
	free(pWorkMem);
	return true;
}

