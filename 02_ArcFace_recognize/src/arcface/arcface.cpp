/*
 * arcface.cpp
 *
 *  Created on: Aug 20, 2018
 *      Author: cui
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <vector>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/time.h>

#include "merror.h"
#include "arcface.h"
#include "config.h"
#include "joinus.h"

#define DBG_EN 	0
#if (DBG_EN == 1)
#define printf_dbg(...) 	fprintf(stderr, "[arcface_dbg](%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)
#else
#define printf_dbg(...)
#endif

#define printf_info(...) 	fprintf(stderr, "[arcface_info](%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)
#define printf_warn(...) 	fprintf(stderr, "[arcface_warn](%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)
#define printf_err(...)  	fprintf(stderr, "[arcface_err](%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)

arcface::arcface()
{
	pWorkMem_FD = nullptr;
	pWorkMem_FR = nullptr;
	hEngine_FD	= nullptr;
	hEngine_FR	= nullptr;

	max_face_number = 50;
	nscale = 16;

	pWorkMem_FD = (MByte *)malloc(FD_WORKBUF_SIZE);
	if(pWorkMem_FD == nullptr){
		err_quit("face detect initial error: fail to malloc workbuf\r\n");
	}

	pWorkMem_FR = (MByte *)malloc(FR_WORKBUF_SIZE);
	if(pWorkMem_FR == nullptr){
		err_quit("face recognize initial error: fail to malloc workbuf\r\n");
	}
}

arcface::~arcface()
{
	if(nullptr != pWorkMem_FD) free(pWorkMem_FD);
	if(nullptr != pWorkMem_FR) free(pWorkMem_FR);
	for(unsigned int i = 0; i < face_models_vec.size(); i++){
		if(face_models_vec.at(i).pbFeature == nullptr)
		{
			free(face_models_vec.at(i).pbFeature);
			face_models_vec.at(i).pbFeature = nullptr;
		}
	}
}


/*************************************************
函数名	: 		arc_init
描述		: 		初始化arcsoft引擎

输入参数	:
		无
输出参数	：
		无

返回值	:		bool
其他		:		无
*************************************************/
bool arcface::arc_init()
{
	int ret = AFD_FSDK_InitialFaceEngine( (const MPChar)APPID, (const MPChar)FD_SDKKEY, pWorkMem_FD, FD_WORKBUF_SIZE,
										 &hEngine_FD, AFD_FSDK_OPF_0_HIGHER_EXT, nscale, max_face_number );
	if (ret != 0) {
		switch(ret){
		case MERR_INVALID_PARAM:
			printf_err( "Face detect AFD_FSDK_InitialFaceEngine(): 0x%x  invalid parameter\r\n", ret );
			break;
		case MERR_NO_MEMORY:
			printf_err( "Face detect AFD_FSDK_InitialFaceEngine(): 0x%x  no memory\r\n", ret );
			break;
		default:
			printf_err( "Face detect fail to AFD_FSDK_InitialFaceEngine(): 0x%x\r\n", ret);
		}
		free(pWorkMem_FD);
		return false;
	}
	printf_dbg( "face detect init success\n" );

	ret = AFR_FSDK_InitialEngine( (const MPChar)APPID, (const MPChar)FR_SDKKEY, pWorkMem_FR, FR_WORKBUF_SIZE,
										 &hEngine_FR );
	if (ret != 0) {
		switch(ret){
		case MERR_INVALID_PARAM:
			printf_err( "Face recognize AFD_FSDK_InitialFaceEngine(): 0x%x  invalid parameter\r\n", ret );
			break;
		case MERR_NO_MEMORY:
			printf_err( "Face recognize AFD_FSDK_InitialFaceEngine(): 0x%x  no memory\r\n", ret );
			break;
		default:
			printf_err( "Face recognize fail to AFD_FSDK_InitialFaceEngine(): 0x%x\r\n", ret);
		}
		free(pWorkMem_FR);
		return false;
	}
	printf_dbg( "face recognize init success\n" );
	return true;
}


/*************************************************
函数名	: 		arc_stop
描述		: 		关闭人脸库引擎

输入参数	:
		无
输出参数	：
		无

返回值	:		无
其他		:		无
*************************************************/
void arcface::arc_stop()
{
    int ret = AFD_FSDK_UninitialFaceEngine(hEngine_FD);
	if( ret != MOK ){
		switch(ret){
		case MERR_INVALID_PARAM:
			printf_err( "AFD_FSDK_UninitialFaceEngine() error--invalid parameter: 0x%x\r\n", ret );
			break;
		default:
			printf_err( "fail to AFD_FSDK_UninitialFaceEngine(): 0x%x\r\n", ret );
		}
	}
	cout<<"uninit fd ret:"<<ret<<endl;

	ret = AFR_FSDK_UninitialEngine(hEngine_FR);
	if( ret != MOK ){
		switch(ret){
		case MERR_INVALID_PARAM:
			printf_err( "AFD_FSDK_UninitialFaceEngine() error--invalid parameter: 0x%x\r\n", ret );
			break;
		default:
			printf_err( "fail to AFD_FSDK_UninitialFaceEngine(): 0x%x\r\n", ret );
		}
	}
	cout<<"uninit fr ret:"<<ret<<endl;

}


/*************************************************
函数名	: 		arcface_getversion
描述		: 		获取人脸库信息并输出

输入参数	:
		无
输出参数	：
		无

返回值	:		无
其他		:		无
*************************************************/
void arcface::arcface_getversion()
{
	const AFD_FSDK_Version*	pVersionInfo = AFD_FSDK_GetVersion(hEngine_FD);
	printf("\n##########################Face detect#################################\n"
			"Codebace:%d\tMajor:%d\tMinor:%d\tBuild:%d\r\n", pVersionInfo->lCodebase, pVersionInfo->lMajor,
								 pVersionInfo->lMinor, pVersionInfo->lBuild);
	printf("Version:%s\r\n", pVersionInfo->Version);
	printf("Buildtime:%s\r\n", pVersionInfo->BuildDate);
	printf("CopyRight:%s\r\n", pVersionInfo->CopyRight);
	printf( "##########################Face detect#################################\n" );

	pVersionInfo = AFD_FSDK_GetVersion(hEngine_FR);
	printf("\n##########################Face recognize#################################\n"
			"Face recognize:\nCodebace:%d\tMajor:%d\tMinor:%d\tBuild:%d\r\n", pVersionInfo->lCodebase, pVersionInfo->lMajor,
								 pVersionInfo->lMinor, pVersionInfo->lBuild);
	printf("Version:%s\r\n", pVersionInfo->Version);
	printf("Buildtime:%s\r\n", pVersionInfo->BuildDate);
	printf("CopyRight:%s\r\n", pVersionInfo->CopyRight);
	printf( "##########################Face recognize#################################\n\n" );
}


/*************************************************
函数名	: 		build_feature_list
描述		: 		创建人脸库特征点列表

输入参数	:
		string path					(in)	创建人脸库的文件夹路径
输出参数	：
		无

返回值	:		ret		暂无处理
其他		:		打印出获取人脸位置信息和获取人脸特征点花费的时间
*************************************************/
int arcface::build_feature_list( string path )
{
	int ret = 0;
	//1st load image from dir
	_load_faces_imgs(path);
	//2nd convert mat to asv
	bool result = _convert_mat_to_asvl(srcimg_mat_vec,dst_asvimg_vec);
	if( false == result ){
		printf_err( "convert img_mat to asvl failed\n\n" );
		return -1;
	}

	//3rd retrieve face xy
	struct timeval start;
	struct timeval end;
	//struct timezone tz; //后面有说明
	gettimeofday(&start,NULL); //gettimeofday(&start,&tz);结果一样
	ret = _get_faces_infos(hEngine_FD,dst_asvimg_vec, face_xy_vec);
	gettimeofday(&end,NULL);
	printf_info("Get faces info time elapsed  %f s\n",
			((float)(end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec))/1000000.0);

	//4th extract features
	gettimeofday(&start,NULL);
	ret = _get_faces_features(hEngine_FR, dst_asvimg_vec, face_xy_vec, face_models_vec);
	gettimeofday(&end,NULL);
	printf_info("Get face features time elapsed  %f s\n",
			((float)(end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec))/1000000.0);

	return ret;
}

/*************************************************
函数名	: 		get_faceID_score
描述		: 		获取待检测图片在人脸库模型中的最大分值和ID号

输入参数	:
		Mat& img					(in)	需要识别的图片
输出参数	：
		无

返回值	:		face_idx_score		在人脸库中的ID号和相似分数值
其他		:		无
*************************************************/
face_idx_score	arcface::get_faceID_score( Mat& img )
{
    MFloat score = 0.0f, max = 0.0f;
    int index = 0;
    face_idx_score result_idx_score;
    //convert mat to asv
    vector<Mat> 	targetMat;
    vector<Mat>& 	rTargetMat = targetMat;
    vector<ASVLOFFSCREEN> targetAsv;
    vector<ASVLOFFSCREEN>& rTargetAsv = targetAsv;
    targetMat.push_back(img);
    LPAFD_FSDK_FACERES tempFaceResult;
    LPAFD_FSDK_FACERES& rtempFaceResult = tempFaceResult;

    AFR_FSDK_FACEINPUT faceXy;
    AFR_FSDK_FACEMODEL tempLocalFaceModels = { 0 }, targetFaceModels = {0};


    _convert_mat_to_asvl(rTargetMat,rTargetAsv);
    _get_face_info(hEngine_FD,&(rTargetAsv.at(0)),&tempFaceResult);
    int idx = _get_the_biggest_face_idx(rtempFaceResult);

    faceXy.lOrient = AFR_FSDK_FOC_0;
    faceXy.rcFace.left = tempFaceResult->rcFace[idx].left;
    faceXy.rcFace.top = tempFaceResult->rcFace[idx].top;
    faceXy.rcFace.right = tempFaceResult->rcFace[idx].right;
    faceXy.rcFace.bottom = tempFaceResult->rcFace[idx].bottom;

    _get_face_feature(hEngine_FR,&(rTargetAsv.at(0)),&faceXy,&tempLocalFaceModels);
    targetFaceModels.lFeatureSize = tempLocalFaceModels.lFeatureSize;
    targetFaceModels.pbFeature = (MByte*)malloc(targetFaceModels.lFeatureSize);
    memcpy(targetFaceModels.pbFeature, tempLocalFaceModels.pbFeature, targetFaceModels.lFeatureSize);

    //
    for(unsigned int i=0;i<face_models_vec.size();i++) {
        int ret = AFR_FSDK_FacePairMatching(hEngine_FR,&face_models_vec.at(i),&targetFaceModels,&score);
        if( ret != MOK ){
        	printf_err( "AFR_FSDK_FacePairMatching error: 0x%x\r\n", ret  );
        }
        if(score >= max){
            max = score;
            index = i;
        }
    }
    result_idx_score.idx = index;
    result_idx_score.score = max;
    free(targetFaceModels.pbFeature);
    cout<<"face ID is: "<< index << endl;
    cout<<"face score is: "<< max << endl;
    return result_idx_score;
}


/*************************************************
函数名	: 		_load_faces_imgs
描述		: 		加载指定路径下的图像

输入参数	:
		string path					(in)	需要加载的文件夹路径
输出参数	：
		无

返回值	:		bool
其他		:		filenames_vec	私有数据，文件名的字符串向量被赋值
				srcimg_mat_vec	原始图像数据存储到私有数据，矩阵向量中
*************************************************/
bool arcface::_load_faces_imgs(string path)
{
    vector<string> names = list_target_files(path);
    filenames_vec = names;
    for(unsigned int i=0; i<names.size();i++){
        Mat tempMat;
        String fullpath = path+"/"+names.at(i);
        tempMat = imread(fullpath);
        if(!tempMat.empty()) {
        	srcimg_mat_vec.push_back(tempMat);
        }else{
            cout<<"this file "<<fullpath<<" can not loaded"<<endl;
        }
    }
    printf_dbg( "_load_faces_imgs success\n" );
    return true;
}


/*************************************************
函数名	: 		list_target_files
描述		: 		列出目标文件夹中的文件名

输入参数	:
		string fileFolderPath					(in)	需要列出文件名的文件夹路径
输出参数	：
		无

返回值	:		vector<string>		返回包含文件名的字符串向量
其他		:		无
*************************************************/
vector<string> arcface::list_target_files(string fileFolderPath)
{
    struct dirent *ptr;
    DIR *dir;
    string PATH = fileFolderPath;
    dir = opendir(PATH.c_str());
    vector<string> files;

    while( (ptr = readdir(dir)) != NULL ){
        if( ptr->d_name[0] == '.'){
            continue;
        }
        files.push_back(ptr->d_name);
    }

    printf_dbg( "Source picture names list:\n" );
    for(unsigned int i = 0; i < files.size(); i++){
        cout << files[i] << endl;
    }

    closedir(dir);
    return files;
}


/*************************************************
函数名	: 		_convert_mat_to_asvl
描述		: 		将图片矩阵转换为arcsoft可识别的数据格式

输入参数	:
		vector<Mat>& srcVec					(in)	待转换图片的向量矩阵
输出参数	：
		vector<ASVLOFFSCREEN>& targetVec	(out)	arcsoft可识别的向量数据集

返回值	:		true/false
其他		:		测试使用数据格式：RGB24_B8G8R8	时，人脸匹配度低一点(0.674384)
				使用I420格式时，匹配度高一点(0.707299)
				NV12:待验证
        		NV21:待验证
        		YUYV:待验证
*************************************************/
bool arcface::_convert_mat_to_asvl(vector<Mat>& srcVec,
                    vector<ASVLOFFSCREEN>& targetVec)
{

    bool ret = true;
    for (unsigned int i = 0; i < srcVec.size(); i++) {
        ASVLOFFSCREEN inputImg = {0};
        inputImg.u32PixelArrayFormat = PIXEL_FORMAT;
        inputImg.i32Width = srcVec.at(i).cols;
        inputImg.i32Height = srcVec.at(i).rows;

        switch( inputImg.u32PixelArrayFormat ){
        case RGB24_B8G8R8:
        	inputImg.ppu8Plane[0] = (MUInt8*)srcVec.at(i).data;
        	break;
        case I420:
        	try{
				cvtColor( srcVec.at(i), srcVec.at(i), CV_BGR2YUV_I420);
				inputImg.ppu8Plane[0] = (MUInt8*)srcVec.at(i).data;
			}catch(...){
				cout << "convert error, please check the file:"<<filenames_vec.at(i) << endl;
				ret = false;
				goto CVT_MAT_2_ASVL;
			}
			break;
        case NV12:
        case NV21:
        case YUYV:
        default:
        	printf_err( "Please select right format\n" );
        	ret = false;
			goto CVT_MAT_2_ASVL;
        }

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
            ret = false;
            goto CVT_MAT_2_ASVL;
        }
        targetVec.push_back(inputImg);
    }
    CVT_MAT_2_ASVL:
        return ret;
}


/*************************************************
函数名	: 		_get_face_info
描述		: 		获取图片中的人脸矩形位置信息

输入参数	:
		MHandle h						(in)	人脸检测引擎
		ASVLOFFSCREEN* pImg				(in)	待检测图片
输出参数	：
		LPAFD_FSDK_FACERES *pResult		(out)	图片中检测到的人脸矩阵信息

返回值	:		ret
其他		:		无
*************************************************/
int arcface::_get_face_info(MHandle h,ASVLOFFSCREEN* pImg, LPAFD_FSDK_FACERES *pResult)
{
    int ret = AFD_FSDK_StillImageFaceDetection(h,pImg,pResult);
    if( ret != MOK ) {
		printf_err( "fail to AFD_FSDK_StillImageFaceDetection(): 0x%x\r\n", ret);
		return ret;
	}

	for (int i = 0; i < (*pResult)->nFace; i++) {
	printf_dbg("face %d:(%d,%d,%d,%d)\r\n", i,
			(*pResult)->rcFace[i].left, (*pResult)->rcFace[i].top,
			(*pResult)->rcFace[i].right, (*pResult)->rcFace[i].bottom);

//	rectangle(frame, Point(faceResult->rcFace[i].left, faceResult->rcFace[i].top),
//		Point(faceResult->rcFace[i].right, faceResult->rcFace[i].bottom),
//		Scalar(255, 0, 0), 2);
	}
    return ret;
}


/*************************************************
函数名	: 		_get_faces_infos
描述		: 		获取图片向量数组中，每张图片最大的人脸矩形位置

输入参数	:
		MHandle h						(in)	人脸检测引擎
		vector<ASVLOFFSCREEN>& imgs		(in)	待检测图片向量数组
输出参数	：
		vector<face_xy>& results		(out)	每张图片最大人脸矩阵信息向量数组

返回值	:		0
其他		:		无
*************************************************/
int arcface::_get_faces_infos(MHandle h,
                                vector<ASVLOFFSCREEN>& imgs,
                                vector<face_xy>& results)
{
	results.clear();
    int ret = 0;
    for(unsigned int i=0; i<imgs.size(); i++)
    {
        LPAFD_FSDK_FACERES faceResult;
        LPAFD_FSDK_FACERES& rfaceResult = faceResult;
        face_xy xypoint;
        int index = 0;
        ret = _get_face_info(h,&(imgs.at(i)),&faceResult);
        index = _get_the_biggest_face_idx(rfaceResult);
        xypoint.top = faceResult->rcFace[index].top;
        xypoint.left = faceResult->rcFace[index].left;
        xypoint.bottom = faceResult->rcFace[index].bottom;
        xypoint.right = faceResult->rcFace[index].right;
        results.push_back(xypoint);
    }
    return ret;
}


/*************************************************
函数名	: 		_get_the_biggest_face_idx
描述		: 		获取一张图片人脸检测结果中，最大的人脸矩形位置

输入参数	:
		LPAFD_FSDK_FACERES& pResult		(in)	图片的人脸检测结果
输出参数	：
		无

返回值	:		矩形框索引值
其他		:		无
*************************************************/
int arcface::_get_the_biggest_face_idx(LPAFD_FSDK_FACERES& pResult)
{
    int max = 0, index = 0;
    for(int i = 0; i < pResult->nFace; i++)
    {
        int tmpV = abs(pResult->rcFace[i].left - pResult->rcFace[i].right);
        if(tmpV >= max)
        {
            max = tmpV;
            index = i;
        }
    }
    return index;
}


/*************************************************
函数名	: 		_get_face_feature
描述		: 		提取图像中人脸特征点

输入参数	:
		MHandle h								(in)	arcsoft引擎
		ASVLOFFSCREEN* pImg						(in)	待提取特征点的图像
		AFR_FSDK_FACEINPUT* pXy					(in)	图像中人脸矩形位置
输出参数	：
		AFR_FSDK_FACEMODEL* pFm					(out)	人脸特征点

返回值	:		ret
其他		:		无
*************************************************/
int arcface::_get_face_feature(MHandle h,
                                ASVLOFFSCREEN* pImg,
                                AFR_FSDK_FACEINPUT* pXy,
                                AFR_FSDK_FACEMODEL* pFm)
{
    int ret = AFR_FSDK_ExtractFRFeature(h,pImg,pXy,pFm);
    if( ret != MOK ) {
		printf_err( "fail to AFR_FSDK_ExtractFRFeature(): 0x%x\r\n", ret);
		return ret;
	}
    return ret;
}


/*************************************************
函数名	: 		_get_faces_features
描述		: 		获取图像库的特征点列表

输入参数	:
		MHandle h								(in)	arcsoft引擎
		vector<ASVLOFFSCREEN>& imgVec			(in)	待提取特征点的图像库
		vector<face_xy>& results				(in)	图像库中人脸矩形位置
输出参数	：
		vector<AFR_FSDK_FACEMODEL>& faceMVec	(out)	人脸特征模型向量

返回值	:		ret
其他		:		无
*************************************************/
int arcface::_get_faces_features(MHandle h,
                                vector<ASVLOFFSCREEN>& imgVec,
                                vector<face_xy>& results,
                                vector<AFR_FSDK_FACEMODEL>& faceMVec)
{
    int ret = 0;
    for(unsigned int i=0; i < imgVec.size(); i++){
        float per = (i+1.00001f)/imgVec.size();
        int p = (int)(per*100.0f);
        cout<<flush<<'\r'<<" "<<to_string(p)<<"/"<<100<<" "<<string(p/10,'#')<<endl;
        //int idx = _get_the_biggest_face_idx(&results.at(i));
        AFR_FSDK_FACEINPUT faceXy;
        AFR_FSDK_FACEMODEL LocalFaceModels = { 0 }, targetFaceModels = {0};

        faceXy.lOrient = AFR_FSDK_FOC_0;
        faceXy.rcFace.left = results.at(i).left;
        faceXy.rcFace.top = results.at(i).top;
        faceXy.rcFace.right = results.at(i).right;
        faceXy.rcFace.bottom = results.at(i).bottom;

        ret = _get_face_feature(h, &(imgVec.at(i)), &faceXy, &LocalFaceModels);
        targetFaceModels.lFeatureSize = LocalFaceModels.lFeatureSize;
        if(0 == ret){
            targetFaceModels.pbFeature = (MByte*)malloc(targetFaceModels.lFeatureSize);
            memcpy(targetFaceModels.pbFeature,LocalFaceModels.pbFeature,targetFaceModels.lFeatureSize);
        }else{
            targetFaceModels.pbFeature = nullptr;
            cout<<"get face feature failed, set feature to NULL"<<endl;
        }
        faceMVec.push_back(targetFaceModels);
    }
    return ret;
}
