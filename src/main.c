/*
 ============================================================================
 Name        : main.c
 Author      : john
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "include/threshold.h"
#include "include/process.h"
#include "include/utils.h"
/*============================================================================
 =  代码内容：最大熵阈值分割
 =  修改日期:2009-3-3
 =  作者:crond123
 =  博客:http://blog.csdn.net/crond123/
 =  E_Mail:crond123@163.com
 ===============================================================================*/


int main(void) {
	IplImage *pImg1,*pImg2,*pImg3;//声明IplImage指针
	//载入图像
	pImg1 = cvLoadImage("c:/lena.png", CV_LOAD_IMAGE_GRAYSCALE);
	pImg2 = cvLoadImage("c:/lena.png", CV_LOAD_IMAGE_GRAYSCALE);
	pImg3 = cvLoadImage("c:/lena.png", CV_LOAD_IMAGE_COLOR);

	testBGR2GRAYHistogram(pImg3);

   // 最大熵 阀值测试
	cvThreshold(pImg1, pImg1, (double) thresholdBYMaxEntropy(pImg1), 255, CV_THRESH_BINARY);
	cvNamedWindow("MaxEntropy", CV_WINDOW_AUTOSIZE); //创建窗口
	cvShowImage("MaxEntropy", pImg1); //显示图像

	//大律法 阀值测试
	cvThreshold(pImg2, pImg2, (double) thresholdByOTSU(pImg2), 255, CV_THRESH_BINARY);
	cvNamedWindow("OSTU", CV_WINDOW_AUTOSIZE);//创建窗口
	cvShowImage("OSTU", pImg2); //显示图像

	cvWaitKey(0); //等待按键

	cvDestroyWindow("MaxEntropy"); //销毁窗口
	cvDestroyWindow("OSTU"); //销毁窗口
	cvReleaseImage(&pImg1);
	cvReleaseImage(&pImg2);
	printf("---------------------------");
	return EXIT_SUCCESS;
}

