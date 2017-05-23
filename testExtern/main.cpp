
#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <ctype.h>
#define PI 3.14159265


int main()
{
	IplImage *src = 0;  // source imagre
	IplImage *histimg = 0; // histogram image 
	CvHistogram *hist = 0; // define multi_demention histogram
	IplImage* canny;
	CvMat* canny_m;
	IplImage* dx; // the sobel x difference 
	IplImage* dy; // the sobel y difference 
	CvMat* gradient; // value of gradient
	CvMat* gradient_dir; // direction of gradient
	CvMat* dx_m; // format transform to matrix
	CvMat* dy_m;
	CvMat* mask;
	CvSize  size;
	IplImage* gradient_im;
	int i, j;
	float theta;

	int hdims = 64;     // ����HIST�ĸ�����Խ��Խ��ȷ
	float hranges_arr[] = { -PI / 2, PI / 2 }; // ֱ��ͼ���Ͻ���½�
	float* hranges = hranges_arr;


	float max_val;  // 
	int bin_w;

	src = cvLoadImage("D:/gxh.jpg", 0); // force to gray image
	if (src == 0) return -1;

	cvNamedWindow("Histogram", 0);
	//cvNamedWindow( "src", 0);
	size = cvGetSize(src);
	canny = cvCreateImage(cvGetSize(src), 8, 1);//��Եͼ��
	dx = cvCreateImage(cvGetSize(src), 32, 1);//x�����ϵĲ��//�˴�����������Ϊ8U ���������
	dy = cvCreateImage(cvGetSize(src), 32, 1);
	gradient_im = cvCreateImage(cvGetSize(src), 32, 1);//�ݶ�ͼ��
	canny_m = cvCreateMat(size.height, size.width, CV_32FC1);//��Ե����
	dx_m = cvCreateMat(size.height, size.width, CV_32FC1);
	dy_m = cvCreateMat(size.height, size.width, CV_32FC1);
	gradient = cvCreateMat(size.height, size.width, CV_32FC1);//�ݶȾ���
	gradient_dir = cvCreateMat(size.height, size.width, CV_32FC1);//�ݶȷ������
	mask = cvCreateMat(size.height, size.width, CV_32FC1);//����

	cvCanny(src, canny, 60, 180, 3);//��Ե���
	cvConvert(canny, canny_m);//��ͼ��ת��Ϊ����
	cvSobel(src, dx, 1, 0, 3);// һ��X�����ͼ���� :dx
	cvSobel(src, dy, 0, 1, 3);// һ��Y�����ͼ���� :dy
	cvConvert(dx, dx_m);
	cvConvert(dy, dy_m);
	cvAdd(dx_m, dy_m, gradient); // value of gradient//�ݶȲ��ǵ��ڸ�����x�ĵ�����ƽ������y������ƽ����
	cvDiv(dx_m, dy_m, gradient_dir); // direction
	for (i = 0; i < size.height; i++)
		for (j = 0; j < size.width; j++)
		{
			if (cvmGet(canny_m, i, j) != 0 && cvmGet(dx_m, i, j) != 0)//������ʲô��˼��ֻ����Ե�ϵķ���
			{
				theta = cvmGet(gradient_dir, i, j);
				theta = atan(theta);
				cvmSet(gradient_dir, i, j, theta);
			}
			else
			{
				cvmSet(gradient_dir, i, j, 0);
			}

		}


	hist = cvCreateHist(1, &hdims, CV_HIST_ARRAY, &hranges, 1);  // ����һ��ָ���ߴ��ֱ��ͼ�������ش�����ֱ��ͼָ��
	histimg = cvCreateImage(cvSize(320, 200), 8, 3); // ����һ��ͼ��3ͨ��
	cvZero(histimg); // ��0��
	cvConvert(gradient_dir, gradient_im);//���ݶȷ������ת��Ϊͼ��
	cvCalcHist(&gradient_im, hist, 0, canny); // ����ֱ��ͼ
	cvGetMinMaxHistValue(hist, 0, &max_val, 0, 0);  // ֻ�����ֵ
	cvConvertScale(hist->bins, hist->bins, max_val ? 255. / max_val : 0., 0); // ���� bin ������ [0,255] ������ϵ��
	cvZero(histimg);
	bin_w = histimg->width / 16;  // hdims: ���ĸ������� bin_w Ϊ���Ŀ��

	// ��ֱ��ͼ
	for (i = 0; i < hdims; i++)
	{
		double val = (cvGetReal1D(hist->bins, i)*histimg->height / 255);// ���ص�ͨ�������ָ��Ԫ��
		// ����ֱ��ͼ��i���Ĵ�С��valΪhistimg�е�i���ĸ߶�
		CvScalar color = CV_RGB(255, 255, 0); //(hsv2rgb(i*180.f/hdims);//ֱ��ͼ��ɫ
		cvRectangle(histimg, cvPoint(100 + i*bin_w, histimg->height), cvPoint(100 + (i + 1)*bin_w, (int)(histimg->height - val)),
			color, 1, 8, 0); // ��ֱ��ͼ���������Σ����½ǣ����Ͻ�����
	}

	//cvShowImage( "src", src);
	cvShowImage("Histogram", histimg);
	cvWaitKey(0);

	cvDestroyWindow("src");
	cvDestroyWindow("Histogram");
	cvReleaseImage(&src);
	cvReleaseImage(&histimg);
	cvReleaseHist(&hist);

	return 0;
}

