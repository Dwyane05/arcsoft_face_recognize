/*
 * arcface.h
 *
 *  Created on: Aug 20, 2018
 *      Author: cui
 */

#ifndef _ARCFACE_H_
#define _ARCFACE_H_

#include <opencv2/opencv.hpp>
#include "arcsoft_fsdk_face_detection.h"
#include "arcsoft_fsdk_face_recognition.h"
#include "face_typedef.h"

#define FD_WORKBUF_SIZE        (40*1024*1024)
#define FR_WORKBUF_SIZE        (40*1024*1024)
#define MAX_FACE_NUM     	   (50)

using namespace cv;
using namespace std;

class arcface{
private:
	MByte*							pWorkMem_FD;
	MByte*							pWorkMem_FR;
	MHandle 						hEngine_FD;
	MHandle 						hEngine_FR;

	vector<face_xy>					face_xy_vec;
	vector<string>					filenames_vec;
	vector<Mat>						srcimg_mat_vec;
	vector<ASVLOFFSCREEN>			dst_asvimg_vec;
	vector<AFR_FSDK_FACEMODEL> 		face_models_vec;
	vector<LPAFD_FSDK_FACERES>		face_result_vec;

	//用户期望引擎最多能检测出的人脸数 有效值范围[1,50]
	int max_face_number;

//	用于数值表示的最小人脸尺寸 有效值范围[2,50] 推荐值 16。
//	该尺寸是人脸相对于所在图片的长边的占比。例如,如果用户想检测到的最小
//	人脸尺寸是图片长度的 1/8,那么这个 nScale 就应该设置为 8
	int nscale;

	//funcs
	bool            _load_faces_imgs(string path);
	bool            _convert_mat_to_asvl(vector<Mat>& srcVec,
									vector<ASVLOFFSCREEN>& targetVec);
	int             _get_face_info(MHandle h,
									ASVLOFFSCREEN* pImg,
									LPAFD_FSDK_FACERES* pResult);
	int             _get_faces_infos(MHandle h,
									vector<ASVLOFFSCREEN>& imgs,
									vector<face_xy>& results);
	int             _get_face_feature(MHandle h,
									ASVLOFFSCREEN* pImg,
									AFR_FSDK_FACEINPUT* pXy,
									AFR_FSDK_FACEMODEL* pFm);
	int             _get_faces_features(MHandle h,
									vector<ASVLOFFSCREEN>& imgVec,
									vector<face_xy>& results,
									vector<AFR_FSDK_FACEMODEL>& faceMVec);
	int             _get_the_biggest_face_idx(LPAFD_FSDK_FACERES& pResult);

public:
	arcface();
    ~arcface();
    bool    arc_init();
    void    arc_stop();
    int     build_feature_list(string path);
    face_idx_score     get_faceID_score(Mat& img);
//    vector<string>     GetFileNameVec(){return filenames_vec;};
    vector<string>     list_target_files(string path);
    void arcface_getversion();
};



#endif /* 02_ARCFACE_RECOGNIZE_SRC_ARCFACE_ARCFACE_H_ */
