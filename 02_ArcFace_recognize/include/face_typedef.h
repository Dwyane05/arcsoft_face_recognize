/*
 * face_typedef.h
 *
 *  Created on: Aug 20, 2018
 *      Author: cui
 */

#ifndef _FACE_TYPEDEF_H_
#define _FACE_TYPEDEF_H_

#define TYPE_MAT 0
#define TYPE_ASVL 1

typedef struct
{
    int idx = 0;
    float score = 0.0f;
}face_idx_score;

typedef struct
{
    int top = 0;
    int left = 0;
    int bottom = 0;
    int right = 0;
}face_xy;



#endif /* 02_ARCFACE_RECOGNIZE_INCLUDE_FACE_TYPEDEF_H_ */
