#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <assert.h>



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

int fu_ReadFile(const char* path, uint8_t **raw_data, size_t* pSize) {
    int res = 0;
    FILE *fp = 0;
    uint8_t *data_file = 0;
    size_t size = 0;

    fp = fopen(path, "rb");
    if (fp == nullptr) {
        res = -1;
        goto exit;
    }

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    data_file = (uint8_t *)malloc(sizeof(uint8_t)* size);
    if (data_file == nullptr) {
        res = -2;
        goto exit;
    }

    if (size != fread(data_file, sizeof(uint8_t), size, fp)) {
        res = -3;
        goto exit;
    }

    *raw_data = data_file;
    data_file = nullptr;
exit:
    if (fp != nullptr) {
        fclose(fp);
    }

    if (data_file != nullptr) {
        free(data_file);
    }

    if (nullptr != pSize) {
        *pSize = size;
    }
    
    return res;
}


int main(int argc, char* argv[]) {

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

    ASVLOFFSCREEN inputImg = { 0 };
    inputImg.u32PixelArrayFormat = INPUT_IMAGE_FORMAT;
    inputImg.i32Width = INPUT_IMAGE_WIDTH;
    inputImg.i32Height = INPUT_IMAGE_HEIGHT;
    inputImg.ppu8Plane[0] = nullptr;
    fu_ReadFile(INPUT_IMAGE_PATH, (uint8_t**)&inputImg.ppu8Plane[0], nullptr);
     if (!inputImg.ppu8Plane[0]) {
        fprintf(stderr, "fail to fu_ReadFile(%s): %s\r\n", INPUT_IMAGE_PATH, strerror(errno));
		AFD_FSDK_UninitialFaceEngine(hEngine);
        free(pWorkMem);
        exit(0);
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

    }

    
    free(inputImg.ppu8Plane[0]);
	AFD_FSDK_UninitialFaceEngine(hEngine);
    free(pWorkMem);

    return 0;
}
