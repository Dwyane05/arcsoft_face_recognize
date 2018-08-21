/*
 * main.cpp
 *
 *  Created on: Aug 21, 2018
 *      Author: cui
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <iostream>

#include "arcface.h"

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



int main(int argc, char* argv[]) {
//    int ret = 0;
    face_idx_score result;
    //1st create a class instance
    arcface face_instance;
    //2nd start engine
    bool status = face_instance.arc_init();
    if( true != status ){
    	err_quit( "ArcSoft face initial failed\n" );
    }
    face_instance.arcface_getversion();

    //3rd create a folder, put header imags in, then build the face features
    int ret = face_instance.build_feature_list("./img");
    if( 0 != ret ){
    	printf_err( "Arcsoft build feature list error\n" );
    	face_instance.arc_stop();
    	exit(1);
    }
    //4th load testimg folder imgs to get result
    vector<string> testlist = face_instance.list_target_files("./testimg");
    vector<face_idx_score> testresult;
    for(unsigned int i=0; i<testlist.size();i++)
    {
        Mat tt = imread("./testimg/"+testlist.at(i));
        face_idx_score tis = face_instance.get_faceID_score(tt);
        testresult.push_back(tis);
    }
//    //5 write the result to csv file
//    CSVWriter csv;
//    csv.enableAutoNewRow(3);
//    csv << "被测图像名称" << "识别结果"<<"得分";
//    for(int i =0; i<testresult.size(); i++)
//    {
//        string fname = testlist.at(i);
//        int idx = testresult.at(i).idx;
//        string resultfname = face_instance.GetFileNameVec().at(idx);
//        float sco = testresult.at(i).score;
//        csv << fname << resultfname << sco;
//    }
//    csv.writeToFile("result.csv",true);
    //6th release the engine

    face_instance.arc_stop();
    return 0;
}


