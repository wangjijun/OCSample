/*
 * process.c
 * 图像处理类
 * Created on: 2012-9-26
 * Author: john
 */
#include "../include/process.h"
/*======================================================================
 灰度级图片二值化
 src 图片
 threshold 临界阀值
 ====================================================================== */
void binarizationByOPCV(IplImage *src, int threshold) {
	int i, j, width, height, step, chanel;
	unsigned char *dataSrc;
	width = src->width;
	height = src->height;
	dataSrc = (unsigned char *) src->imageData;
	step = src->widthStep / sizeof(char);
	chanel = src->nChannels;
	for (i = 0; i < height; i++)
		for (j = 0; j < width * chanel; j++) {
			if (dataSrc[i * step + j] > threshold) {
				dataSrc[i * step + j] = 255;
			} else {
				dataSrc[i * step + j] = 0;
			}
		}
}

/*======================================================================
 灰度级图片中值去噪
 src 图片
 ====================================================================== */
void denoisingByOPCV(IplImage *src) {
	int height, width, step, channels;
	uchar *data;
	int x, y, c;
	int arrPixel[9];
	int arrLength = 9;
	// 获取图像信息
	height = src->height;
	width = src->width;
	step = src->widthStep;
	channels = src->nChannels;
	data = (uchar *) src->imageData;
	for (y = 1; y < height; y++) {
		for (x = 1; x < width; x++) {
			for (c = 0; c < channels; c++) {
				//data[i * step + j * channels + k] = 255- data[i * step + j * channels + k];
				//以自身为中心选取矩阵，确定与其相邻8个领域
				arrPixel[0] = data[(y - 1) * step + (x - 1) * channels + c];
				arrPixel[1] = data[(y - 1) * step + x * channels + c];
				arrPixel[2] = data[(y - 1) * step + (x + 1) * channels + c];

				arrPixel[3] = data[y * step + (x - 1) * channels + c];
				arrPixel[4] = data[y * step + x * channels + c];
				arrPixel[5] = data[y * step + (x + 1) * channels + c];

				arrPixel[6] = data[(y + 1) * step + (x - 1) * channels + c];
				arrPixel[7] = data[(y + 1) * step + x * channels + c];
				arrPixel[8] = data[(y + 1) * step + (x + 1) * channels + c];

				int i, j, temp;
				temp = arrPixel[4];
				//计算与自身相邻8像素点的差值的绝对值
				for (i = 0; i < arrLength; i++) {
					arrPixel[i] = abs(arrPixel[i] - temp);
				}
				//冒泡排序
				for (i = 0; i < arrLength; i++) {
					for (j = 1; j < arrLength - i; j++) {
						if (arrPixel[j - 1] < arrPixel[j]) {
							temp = arrPixel[j - 1];
							arrPixel[j - 1] = arrPixel[j];
							arrPixel[j] = temp;
						}
					}
				}
				//排序后对前5位累加求平均值
				temp = 0;
				for (i = 0; i < 5; i++) {
					temp += arrPixel[i] / 5;
				}
				data[y * step + x * channels + c] = temp;
			}
		}
	}
}
