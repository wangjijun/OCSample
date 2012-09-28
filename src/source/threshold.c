/*
 * threshold.c
 * 图像阀值计算类
 * Created on: 2012-9-26
 * Author: john
 */
#include "../include/threshold.h"
/*======================================================================
 OTSU 大律法 动态计算灰度图像求阀值
 ====================================================================== */
int thresholdByOTSU(IplImage *image) {
	int w = image->width;
	int h = image->height;

	unsigned char*np; // 图像指针
	unsigned char pixel;
	int thresholdValue = 1; // 阈值
	int ihist[256]; // 图像直方图，256个点

	int i, j, k; // various counters
	int n, n1, n2, gmin, gmax;
	double m1, m2, sum, csum, fmax, sb;

	// 对直方图置零...
	memset(ihist, 0, sizeof(ihist));
	gmin = 255;
	gmax = 0;
	// 生成直方图
	for (i = 0; i < h; i++) {
		np = (unsigned char*) (image->imageData + image->widthStep * i);
		for (j = 0; j < w; j++) {
			pixel = np[j];
			ihist[pixel]++;
			if (pixel > gmax)
				gmax = pixel;
			if (pixel < gmin)
				gmin = pixel;
		}
	}

	// set up everything
	sum = csum = 0.0;
	n = 0;

	for (k = 0; k <= 255; k++) {
		sum += k * ihist[k]; /* x*f(x) 质量矩*/
		n += ihist[k]; /* f(x) 质量 */
	}

	if (!n) {
		// if n has no value, there is problems...
		//fprintf (stderr, "NOT NORMAL thresholdValue = 160\n");
		thresholdValue = 160;
		goto L;
	}

	// do the otsu global thresholding method
	fmax = -1.0;
	n1 = 0;
	for (k = 0; k < 255; k++) {
		n1 += ihist[k];
		if (!n1) {
			continue;
		}
		n2 = n - n1;
		if (n2 == 0) {
			break;
		}
		csum += k * ihist[k];
		m1 = csum / n1;
		m2 = (sum - csum) / n2;
		sb = n1 * n2 * (m1 - m2) * (m1 - m2);
		/* bbg: note: can be optimized. */
		if (sb > fmax) {
			fmax = sb;
			thresholdValue = k;
		}
	}

	L: for (i = 0; i < h; i++) {
		np = (unsigned char*) (image->imageData + image->widthStep * i);
		for (j = 0; j < w; j++) {
			if (np[j] >= thresholdValue)
				np[j] = 255;
			else
				np[j] = 0;
		}
	}
	printf("大律法 阀值[%d]\n", thresholdValue);
	//cout<<"The Threshold of this Image in Otsu is:"<<thresholdValue<<endl;
	return (thresholdValue);
}

/*======================================================================
 加权最大方差 动态计算灰度图像求阀值
 ======================================================================*/
int thresholdBYVariance(IplImage *src) {
	int i, j, width, height, step, chanel, threshold;
	/*size是图像尺寸，svg是灰度直方图均值，va是方差*/
	float size, avg, va, maxVa, p, a, s;
	unsigned char *dataSrc;
	float histogram[256];
	width = src->width;
	height = src->height;
	dataSrc = (unsigned char *) src->imageData;
	step = src->widthStep / sizeof(char);
	chanel = src->nChannels;

	/*计算直方图并归一化histogram*/
	for (i = 0; i < 256; i++) {
		histogram[i] = 0;
	}
	for (i = 0; i < height; i++) {
		for (j = 0; j < width * chanel; j++) {
			//统计直方图每个灰度级出现的次数
			histogram[dataSrc[i * step + j]]++;
		}
	}
	size = width * height;
	for (i = 0; i < 256; i++) {
		//归一化
		histogram[i] /= size;
	}
	/*计算灰度直方图中值和方差*/
	avg = 0;
	for (i = 0; i < 256; i++) {
		//灰度级加权和计算灰度中值
		avg += i * histogram[i];
	}
	va = 0;
	for (i = 0; i < 256; i++) {
		va += fabs(i * i * histogram[i] - avg * avg);
	}
	/*利用加权最大方差求阀值*/
	threshold = 20;
	maxVa = 0;
	p = a = s = 0;
	for (i = 0; i < 256; i++) {
		p += histogram[i];
		a += i * histogram[i];
		s = (avg * p - a) * (avg * p - a) / p / (1 - p);
		if (s > maxVa) {
			threshold = i;
			maxVa = s;
		}
	}
	printf("加权方差 阀值[%d]\n", threshold);
	return threshold;
}

/*======================================================================
 最大熵 动态计算灰度图像求阀值
 ====================================================================== */
typedef enum {
	back, object
} entropy_state;
double caculateCurrentEntropy(CvHistogram * Histogram1, int cur_threshold,
		entropy_state state) {
	int start, end, total;
	start = end = total = 0;
	double cur_entropy = 0.0;
	if (state == back) {
		start = 0;
		end = cur_threshold;
	} else {
		start = cur_threshold;
		end = 256;
	}

	int i, j;
	for (i = start; i < end; i++) {
		total += (int) cvQueryHistValue_1D(Histogram1,i); //查询直方块的值 P304
	}
	for (j = start; j < end; j++) {
		if ((int) cvQueryHistValue_1D(Histogram1,j) == 0) {
			continue;
		}
		double percentage = cvQueryHistValue_1D(Histogram1,j) / total;
		/*熵的定义公式*/
		cur_entropy += -percentage * logf(percentage);
		/*根据泰勒展式去掉高次项得到的熵的近似计算公式
		 cur_entropy += percentage*percentage;*/
	}
	return cur_entropy;
	//    return (1-cur_entropy);
}

//寻找最大熵阈值
int thresholdBYMaxEntropy(IplImage *src) {
//int MaxEntropy(IplImage *src, IplImage *dst) {
	assert(src != NULL);
//	assert(src->depth ==8&& dst->depth ==8);
	assert(src->depth ==8);
	assert(src->nChannels ==1);

	int HistogramBins = 256;
	float HistogramRange1[2] = { 0, 255 };
	float *HistogramRange[1] = { HistogramRange1 };
	CvHistogram *hist = cvCreateHist(1, &HistogramBins, CV_HIST_ARRAY,HistogramRange, 1); //创建一个指定尺寸的直方图
	//参数含义：直方图包含的维数、直方图维数尺寸的数组、直方图的表示格式、方块范围数组、归一化标志
	cvCalcHist(&src, hist, 0, NULL); //计算直方图
	double maxentropy = -1.0;
	int threshold = -1;
	// 循环测试每个分割点，寻找到最大的阈值分割点
	int i;
	for (i = 0; i < HistogramBins; i++) {
		double cur_entropy = caculateCurrentEntropy(hist, i, object)
				+ caculateCurrentEntropy(hist, i, back);
		if (cur_entropy > maxentropy) {
			maxentropy = cur_entropy;
			threshold = i;
		}
	}
	printf("最大熵 阀值[%d]\n", threshold);
	//cvThreshold(src, dst, (double) threshold, 255, CV_THRESH_BINARY);
	cvReleaseHist(&hist);
	return threshold ;
}
