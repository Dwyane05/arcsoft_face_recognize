/*
 * rtsp_face_detect.cpp
 *
 *  Created on: Aug 16, 2018
 *      Author: cui
 */


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <iostream>
#include <sys/time.h>

#include "facetracking.h"

#include <opencv2/opencv.hpp>
//#include <opencv2/imgproc.hpp>
//#include <opencv2/imgcodecs.hpp>
//#include <opencv2/videoio.hpp>
//#include <opencv2/highgui.hpp>
#include "joinus.h"

#define DBG_EN 	1
#if (DBG_EN == 1)
#define printf_dbg(...) 	fprintf(stderr, "[main_dbg](%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)
#else
#define printf_dbg(...)
#endif

#define printf_info(...) 	fprintf(stderr, "[main_info](%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)
#define printf_warn(...) 	fprintf(stderr, "[main_warn](%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)
#define printf_err(...)  	fprintf(stderr, "[main_err](%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)


using namespace cv;
using namespace std;

Mat current_frame;
facetracking ipc0;
//OpenCV 命令行解析器函数
const char * keys =
{
	"{help h usage ? || print this message}"
	"{@video || Video file, if not defined try to use webcamera}"
};

void help(char** av) {
    cout << "The program captures frames from a video file, image sequence (01.jpg, 02.jpg ... 10.jpg) or camera connected to your computer." << endl
         << "Usage:\n" << av[0] << " <video file, image sequence or device number>" << endl
         << "q,Q,esc -- quit" << endl
         << "space   -- save frame" << endl << endl
         << "\tTo capture from a camera pass the device number. To find the device number, try ls /dev/video*" << endl
         << "\texample: " << av[0] << " 0" << endl
         << "\tYou may also pass a video file instead of a device number" << endl
         << "\texample: " << av[0] << " video.avi" << endl
         << "\tYou can also pass the path to an image sequence and OpenCV will treat the sequence just like a video." << endl
         << "\texample: " << av[0] << " right%%02d.jpg" << endl;
}

int process(VideoCapture& capture) {
//	int width = capture.get(CAP_PROP_FRAME_WIDTH);
//	int height = capture.get(CAP_PROP_FRAME_HEIGHT);
	cout << "width = " << capture.get(CAP_PROP_FRAME_WIDTH) << endl;
	cout << "height = " << capture.get(CAP_PROP_FRAME_HEIGHT) << endl;

	long int n = 0;
	char filename[200];
	string window_name = "video | q or esc to quit";
	cout << "press space to save a picture. q or esc to quit" << endl;
//        namedWindow(window_name, WINDOW_KEEPRATIO); //resizable window;namedWindow( "Webcam_org", CV_WINDOW_AUTOSIZE );
	namedWindow(window_name, CV_WINDOW_AUTOSIZE);
	Mat frame;
//	int check = 0;
	struct timeval start;
	struct timeval end;
	gettimeofday(&start,NULL);

	for (;;) {
		capture.read( frame );
		if (frame.empty())
			continue;
//		check = (check + 1)%2;
		if( ipc0.ft_get_inputimage( frame.cols, frame.rows, frame ) ){
			ipc0.ft_detect( frame );
		}

		imshow(window_name, frame);
		waitKey(1);
		gettimeofday(&end,NULL);
		if( (int)(end.tv_sec-start.tv_sec) > 30 )
			break;
//		char key = (char)waitKey(1); //delay N millis, usually long enough to display and capture input
//		switch (key) {
//		case 'q':
//		case 'Q':
//		case 27: //escape key
//			break;
////			return 0;
//		case ' ': //Save an image
//			sprintf(filename,"filename%.3ld.jpg",n++);
//			imwrite(filename,frame);
//			cout << "Saved " << filename << endl;
//			break;
//		default:
//			break;
//		}
	}
	return 0;
}


int main(int argc, char* argv[] ) {
    cv::CommandLineParser parser(argc, argv, keys);
    //如果需要，显示帮助文档
	if( parser.has("help")){
		parser.printMessage();
		return 0;
	}
	std::string videoFile = parser.get<std::string>(0);
	//分析params 的变量，检查params是否正确
	if( !parser.check() ){
		parser.printErrors();
		return 0;
	}

    VideoCapture capture( videoFile ); //try to open string, this will attempt to open it as a video file or image sequence
    if( !capture.isOpened() ) //if this fails, try to open as a video camera, through the use of an integer param
        capture.open(atoi(videoFile.c_str()));
    if( !capture.isOpened() ) {
        help(argv);
        err_quit( "\nFailed to open the video device, video file or image sequence!\n" );
    }


    if( !ipc0.ft_init() ){
    	err_quit( "ipc0 init error\n\n" );
    }

    ipc0.ft_getversion();
    struct timeval start;
	struct timeval end;
	//struct timezone tz; //后面有说明
	gettimeofday(&start,NULL); //gettimeofday(&start,&tz);结果一样
	process(capture);
	gettimeofday(&end,NULL);
	printf_info("Get faces info time elapsed  %f s\n",
			((float)(end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec))/1000000.0);


    ipc0.ft_uninit();

    exit(0);
}

