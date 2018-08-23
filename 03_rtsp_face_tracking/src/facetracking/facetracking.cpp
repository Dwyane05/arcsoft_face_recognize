/*
 * facetracking.cpp
 *
 *  Created on: Aug 22, 2018
 *      Author: cui
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <iostream>

#include "facetracking.h"
#include "merror.h"

#include "config.h"
#include "joinus.h"

using namespace cv;

#define DBG_EN 	1
#if (DBG_EN == 1)
#define printf_dbg(...) 	fprintf(stderr, "[ft_dbg](%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)
#else
#define printf_dbg(...)
#endif

#define printf_info(...) 	fprintf(stderr, "[ft_info](%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)
#define printf_warn(...) 	fprintf(stderr, "[ft_warn](%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)
#define printf_err(...)  	fprintf(stderr, "[ft_err](%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)


facetracking::facetracking()
{
	FT_pWorkMem = nullptr;
	FT_hEngine = nullptr;
	inputImg = { 0 };

	max_face_number = 10;
	nscale = 16;

	FT_pWorkMem = (MByte *)malloc(FT_WORKBUF_SIZE);
	if(FT_pWorkMem == nullptr){
		printf_err("face detect init error: fail to malloc workbuf\r\n");
		exit(-1);
	}

}

facetracking::~facetracking()
{
	if(nullptr != FT_pWorkMem)
		free(FT_pWorkMem);
}

bool facetracking::ft_init()
{
	int ret = AFT_FSDK_InitialFaceEngine((MPChar)APPID, (MPChar)FT_SDKKEY, FT_pWorkMem, FT_WORKBUF_SIZE,
										 &FT_hEngine, AFT_FSDK_OPF_0_HIGHER_EXT, nscale, max_face_number );
	if (ret != 0) {
		switch(ret){
		case MERR_INVALID_PARAM:
			printf_err( "AFT_FSDK_InitialFaceEngine(): 0x%x  invalid parameter\r\n", ret );
			break;
		case MERR_NO_MEMORY:
			printf_err( "AFT_FSDK_InitialFaceEngine(): 0x%x  no memory\r\n", ret );
			break;
		default:
			printf_err( "fail to AFT_FSDK_InitialFaceEngine(): 0x%x\r\n", ret);
		}
		return false;
	}
	printf_dbg( "face tracking init success\n" );
	return true;
}


void facetracking::ft_getversion()
{
	const AFT_FSDK_Version * pVersionInfo = nullptr;
	pVersionInfo = AFT_FSDK_GetVersion(FT_hEngine);
	printf("\n##########################Face tracking#################################\n"
			"Codebace:%d\tMajor:%d\tMinor:%d\tBuild:%d\r\n", pVersionInfo->lCodebase, pVersionInfo->lMajor,
								 pVersionInfo->lMinor, pVersionInfo->lBuild);
	printf("Version:%s\r\n", pVersionInfo->Version);
	printf("Buildtime:%s\r\n", pVersionInfo->BuildDate);
	printf("CopyRight:%s\r\n", pVersionInfo->CopyRight);
	printf( "##########################Face tracking#################################\n\n" );
}


bool  facetracking::ft_get_inputimage( int img_width, int img_height, cv::Mat &frame )
{
	inputImg.u32PixelArrayFormat = PIXEL_FORMAT;
	inputImg.i32Width = img_width;
	inputImg.i32Height = img_height;

//	cvtColor( frame, frame, CV_BGR2YUV_I420);
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
		printf_err( "face detect error: unsupported Image format: 0x%x\r\n",inputImg.u32PixelArrayFormat );
		return false;
//		free(inputImg.ppu8Plane[0]);
//		AFD_FSDK_UninitialFaceEngine(hEngine);
//		free(pWorkMem);
	}
	return true;
}



bool facetracking::ft_detect( cv::Mat &frame )
{
	//根据输入的图像检测人脸，一般用于视频检测，多帧方式
	int ret = AFT_FSDK_FaceFeatureDetect( FT_hEngine, &inputImg, &faceResult);
	if( ret != MOK ) {
		printf_err( "fail to AFT_FSDK_FaceFeatureDetect(): 0x%x\r\n", ret);
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

bool facetracking::ft_uninit()
{

	int ret = AFT_FSDK_UninitialFaceEngine(FT_hEngine);
	if( ret != MOK ){
		switch(ret){
		case MERR_INVALID_PARAM:
			printf_err( "AFT_FSDK_UninitialFaceEngine() error--invalid parameter: 0x%x\r\n", ret );
			break;
		default:
			printf_err( "fail to AFT_FSDK_UninitialFaceEngine(): 0x%x\r\n", ret );
		}
	}
	return true;
}
