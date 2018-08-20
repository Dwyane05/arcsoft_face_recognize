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

#include "facedetect.h"

#include <opencv2/opencv.hpp>
//#include <opencv2/imgproc.hpp>
//#include <opencv2/imgcodecs.hpp>
//#include <opencv2/videoio.hpp>
//#include <opencv2/highgui.hpp>
#include "joinus.h"

using namespace cv;
using namespace std;

Mat current_frame;
facedetect ipc0;
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

	for (;;) {
		capture.read( frame );
		if (frame.empty())
			break;

		if( ipc0.fd_get_inputimage( frame.cols, frame.rows, frame ) ){
			ipc0.fd_detect( frame );
		}

		imshow(window_name, frame);

		char key = (char)waitKey(30); //delay N millis, usually long enough to display and capture input
		switch (key) {
		case 'q':
		case 'Q':
		case 27: //escape key
			return 0;
		case ' ': //Save an image
			sprintf(filename,"filename%.3ld.jpg",n++);
			imwrite(filename,frame);
			cout << "Saved " << filename << endl;
			break;
		default:
			break;
		}
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


    if( !ipc0.facedetect_init() ){
    	err_quit( "ipc0 init error\n\n" );
    }

    ipc0.facedetect_getversion();

    process(capture);

    ipc0.facedetect_uninit();

    exit(0);
}

