#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
using namespace std;

//OpenCV 头文件
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
using namespace cv;

#include "arcsoft_fsdk_face_detection.h"
#include "merror.h"

#define APPID     "EokKppfrCynPrtaSsxFUFDWYurD7zhyxtCMzAfnsFq11"
#define SDKKEY    "7Ybt3PXgYniBxEuDg6etvEJGpard3XEHVgPVxfLHVNgd"


#define INPUT_IMAGE_FORMAT  ASVL_PAF_I420
#define INPUT_IMAGE_PATH    "your_input_image.yuv"
#define INPUT_IMAGE_WIDTH   (640)
#define INPUT_IMAGE_HEIGHT  (480)

#define WORKBUF_SIZE        (40*1024*1024)
#define MAX_FACE_NUM        (50)

//OpenCV 命令行解析器函数
const char * keys =
{
	"{help h usage ? || print this message}"
	"{@video || Video file, if not defined try to use webcamera}"
};

int main(int argc, char* argv[]) {

	CommandLineParser parser( argc, argv, keys );
	parser.about( "Chapter 2. v1.0.0" );

	//如果需要，显示帮助文档
	if( parser.has("help")){
		parser.printMessage();
		return 0;
	}
	String videoFile = parser.get<String>(0);

	//分析params 的变量，检查params是否正确
	if( !parser.check() ){
		parser.printErrors();
		return 0;
	}

	VideoCapture cap;	//打开默认相机
	if( videoFile != "" )
		cap.open( videoFile );
	else
		cap.open(0);
	//检查是否被打开
	if( !cap.isOpened() ){
		fprintf( stderr, "Can't open video \n" );
		return -1;
	}
	namedWindow( "Webcam_org", CV_WINDOW_AUTOSIZE );

    MByte *pWorkMem = (MByte *)malloc(WORKBUF_SIZE);
    if(pWorkMem == nullptr){
        fprintf(stderr, "fail to malloc workbuf\r\n");
        exit(0);
    }

    MHandle hEngine = nullptr;

    int ret = AFD_FSDK_InitialFaceEngine(APPID, SDKKEY, pWorkMem, WORKBUF_SIZE, 
                                         &hEngine, AFD_FSDK_OPF_0_HIGHER_EXT, 16, MAX_FACE_NUM);
    if (ret != 0) {
        fprintf(stderr, "fail to AFD_FSDK_InitialFaceEngine(): 0x%x\r\n", ret);
        free(pWorkMem);
        exit(0);
    }

    const AFD_FSDK_Version*pVersionInfo = AFD_FSDK_GetVersion(hEngine);
    printf("%d %d %d %d\r\n", pVersionInfo->lCodebase, pVersionInfo->lMajor,
                                 pVersionInfo->lMinor, pVersionInfo->lBuild);
    printf("%s\r\n", pVersionInfo->Version);
    printf("%s\r\n", pVersionInfo->BuildDate);
    printf("%s\r\n", pVersionInfo->CopyRight);


	Mat frame;
	while(cap.isOpened() && cap.read(frame) ){
		ASVLOFFSCREEN inputImg = { 0 };
		inputImg.u32PixelArrayFormat = INPUT_IMAGE_FORMAT;
		inputImg.i32Width = frame.cols;
		inputImg.i32Height = frame.rows;
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
			fprintf(stderr, "unsupported Image format: 0x%x\r\n",inputImg.u32PixelArrayFormat);
			free(inputImg.ppu8Plane[0]);
			AFD_FSDK_UninitialFaceEngine(hEngine);
			free(pWorkMem);
			exit(0);
		}
		LPAFD_FSDK_FACERES faceResult;
		ret = AFD_FSDK_StillImageFaceDetection(hEngine, &inputImg, &faceResult);
		if (ret != 0) {
			fprintf(stderr, "fail to AFD_FSDK_StillImageFaceDetection(): 0x%x\r\n", ret);
			free(inputImg.ppu8Plane[0]);
			AFD_FSDK_UninitialFaceEngine(hEngine);
			free(pWorkMem);
			exit(0);
		}
		for (int i = 0; i < faceResult->nFace; i++) {
			printf("face %d:(%d,%d,%d,%d)\r\n", i,
				   faceResult->rcFace[i].left, faceResult->rcFace[i].top,
				   faceResult->rcFace[i].right, faceResult->rcFace[i].bottom);
			rectangle(frame, Point(faceResult->rcFace[i].left, faceResult->rcFace[i].top),
					Point(faceResult->rcFace[i].right, faceResult->rcFace[i].bottom), Scalar(255, 0, 0), 2);
		}

		if( frame.empty()){
			imshow( "Camera 0", frame );
		}

		 if (waitKey(30) > 0) //wait for 'Esc' key press for 30ms. If 'Esc' key is pressed, break loop
		{
			 cout << "Esc key is pressed by user" << endl;
			break;
		}
	}


	cap.release();
	AFD_FSDK_UninitialFaceEngine(hEngine);
    free(pWorkMem);

    return 0;
}
