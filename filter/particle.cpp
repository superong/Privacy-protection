// #include <cv.h>  
// #include <cxcore.h>  
// #include <highgui.h>  
// #include <cvaux.h>  
// 
// #include <cmath>  
// #include <vector>  
// #include <iostream>  
// using namespace std;
// 
// const int winHeight = 600;
// const int winWidth = 800;
// 
// 
// CvPoint mousePosition = cvPoint(winWidth >> 1, winHeight >> 1);//等效于除2
// 
// //mouse event callback  
// void mouseEvent(int event, int x, int y, int flags, void *param)
// {
// 	if (event == CV_EVENT_MOUSEMOVE) {
// 		mousePosition = cvPoint(x, y);
// 	}
// }
// 
// int main(void)
// {
// 	//1.condensation setup  
// 	const int stateNum = 4;//状态变量为4维向量（x,y,Vx,Vy）
// 	const int measureNum = 2;//观测向量为2维向量（x,y）
// 	const int sampleNum = 200;//粒子数量
// 
// 	CvConDensation* condens = cvCreateConDensation(stateNum, measureNum, sampleNum);
// 	CvMat* lowerBound;
// 	CvMat* upperBound;
// 	lowerBound = cvCreateMat(stateNum, 1, CV_32F);//rows,cols:列向量
// 	upperBound = cvCreateMat(stateNum, 1, CV_32F);
// 	/*
// 	lowerBound = [0, 0, 0, 0]';
// 	upperBound=[800,600,0,0]';
// 	*/
// 	cvmSet(lowerBound, 0, 0, 0.0);//设置访问元素
// 	cvmSet(upperBound, 0, 0, winWidth);
// 	cvmSet(lowerBound, 1, 0, 0.0);
// 	cvmSet(upperBound, 1, 0, winHeight);
// 	cvmSet(lowerBound, 2, 0, 0.0);
// 	cvmSet(upperBound, 2, 0, 0.0);
// 	cvmSet(lowerBound, 3, 0, 0.0);
// 	cvmSet(upperBound, 3, 0, 0.0);
// 	
// 	//状态转移矩阵 Xt-1 * A +w = Xt
// 	float A[stateNum][stateNum] = 
// 	{
// 		1, 0, 1, 0,
// 		0, 1, 0, 1,
// 		0, 0, 1, 0,
// 		0, 0, 0, 1
// 	};
// 	memcpy(condens->DynamMatr, A, sizeof(A));//初始化动态转移矩阵
// 	cvConDensInitSampleSet(condens, lowerBound, upperBound);//限定随机数范围
// 
// 	CvRNG rng_state = cvRNG(0xffffffff);//64bits
// 	for (int i = 0; i < sampleNum; i++)//initialize coordinate (x,y) of particles with size of 4*2
// 	{
// 		condens->flSamples[i][0] = float(cvRandInt(&rng_state) % winWidth); //width  
// 		condens->flSamples[i][1] = float(cvRandInt(&rng_state) % winHeight);//height  
// 	}
// 
// 	CvFont font;
// 	cvInitFont(&font, CV_FONT_HERSHEY_SCRIPT_COMPLEX, 1, 1);
// 	
// 	char* winName = "condensation";
// 	cvNamedWindow(winName);
// 	cvSetMouseCallback(winName, mouseEvent);//获取运动目标位置
// 	IplImage* img = cvCreateImage(cvSize(winWidth, winHeight), 8, 3);
// 	bool isPredictOnly = false;//trigger for prediction only,press SPACEBAR  
// 	while (1)
// 	{
// 		//2.condensation prediction  
// 		CvPoint predict_pt = cvPoint((int)condens->State[0], (int)condens->State[1]);
// 		
// 		float variance[measureNum] = { 0 };//观测向量方差初始化
// 		//get variance/standard deviation of each state  
// 		for (int i = 0; i < measureNum; i++) 
// 		{
// 			//sum  
// 			float sumState = 0;
// 			cout << condens->SamplesNum << endl;
// 			for (int j = 0; j < condens->SamplesNum; j++)	
// 			{
// 				sumState += condens->flSamples[j][i];//(row,col)
// 			}
// 			//average  
// 			sumState /= sampleNum;
// 			//variance  
// 			for (int j = 0; j < condens->SamplesNum; j++)
// 			{
// 				variance[i] += (condens->flSamples[j][i] - sumState)*
// 					(condens->flSamples[j][i] - sumState);//平方和
// 			}
// 			variance[i] /= sampleNum - 1;//方差
// 		}
// 		//3.update particals confidence  
// 		CvPoint pt;
// 		if (isPredictOnly) 
// 		{
// 			pt = predict_pt;
// 		}
// 		else
// 		{
// 			pt = mousePosition;//通过观测值来更新权值
// 		}
// 		//当前状态与前一状态的偏离程度来加权
// 		for (int i = 0; i < condens->SamplesNum; i++) 
// 		{
// 			float probX = (float)exp(-1 * (pt.x - condens->flSamples[i][0])
// 				*(pt.x - condens->flSamples[i][0]) / (2 * variance[0]));
// 			float probY = (float)exp(-1 * (pt.y - condens->flSamples[i][1])
// 				*(pt.y - condens->flSamples[i][1]) / (2 * variance[1]));
// 			condens->flConfidence[i] = probX*probY;
// 		}
// 		//4.update condensation  
// 		cvConDensUpdateByTime(condens);
// 
// 		//draw   
// 		cvSet(img, cvScalar(255, 255, 255, 0));
// 		cvCircle(img, predict_pt, 5, CV_RGB(0, 255, 0), 3);//predicted point with green  
// 		char buf[256];
// 		sprintf_s(buf, 256, "predicted position:(%3d,%3d)", predict_pt.x, predict_pt.y);
// 		cvPutText(img, buf, cvPoint(10, 30), &font, CV_RGB(0, 0, 0));
// 		if (!isPredictOnly) {
// 			cvCircle(img, mousePosition, 5, CV_RGB(255, 0, 0), 3);//current position with red  
// 			sprintf_s(buf, 256, "real position :(%3d,%3d)", mousePosition.x, mousePosition.y);
// 			cvPutText(img, buf, cvPoint(10, 60), &font, CV_RGB(0, 0, 0));
// 		}
// 
// 		cvShowImage(winName, img);
// 		int key = cvWaitKey(30);
// 		if (key == 27){//esc     
// 			break;
// 		}
// 		else if (key == ' ') {//trigger for prediction  
// 			//isPredict=!isPredict;  
// 			if (isPredictOnly) {
// 				isPredictOnly = false;
// 			}
// 			else{
// 				isPredictOnly = true;
// 			}
// 		}
// 	}
// 	cout << img->imageData[2];
// 	cvReleaseImage(&img);
// 	cvReleaseConDensation(&condens);
// 	return 0;
// }

// particle_tracking.cpp : 定义控制台应用程序的入口点。
//



#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include <iostream>

using namespace cv;
using namespace std;

Rect select;
bool select_flag = false;
bool tracking = false;//跟踪标志位
bool select_show = false;
Point origin;
Mat frame, hsv;
int after_select_frames = 0;//选择矩形区域完后的帧计数

/****rgb空间用到的变量****/
//int hist_size[]={16,16,16};//rgb空间各维度的bin个数
//float rrange[]={0,255.0};
//float grange[]={0,255.0};
//float brange[]={0,255.0};
//const float *ranges[] ={rrange,grange,brange};//range相当于一个二维数组指针

/****hsv空间用到的变量****/
int hist_size[] = { 16, 16, 16 };
float hrange[] = { 0, 180.0 };
float srange[] = { 0, 256.0 };
float vrange[] = { 0, 256.0 };

//int hist_size[]={32,32,32};
//float hrange[]={0,359.0.0};
//float srange[]={0,1.0};
//float vrange[]={0,1.0};
const float *ranges[] = { hrange, srange, vrange };

int channels[] = { 0, 1, 2 };

/****有关粒子窗口变化用到的相关变量****/
int A1 = 2;
int A2 = -1;
int B0 = 1;
double sigmax = 1.0;
double sigmay = 0.5;
double sigmas = 0.001;

/****定义使用粒子数目宏****/
#define PARTICLE_NUMBER 100 //如果这个数设定太大，经测试这个数字超过25就会报错，则在运行时将会出现错误

/****定义粒子结构体****/
typedef struct particle
{
	int orix, oriy;//原始粒子坐标
	int x, y;//当前粒子的坐标
	double scale;//当前粒子窗口的尺寸
	int prex, prey;//上一帧粒子的坐标
	double prescale;//上一帧粒子窗口的尺寸
	Rect rect;//当前粒子矩形窗口
	Mat hist;//当前粒子窗口直方图特征
	double weight;//当前粒子权值
}PARTICLE;

PARTICLE particles[PARTICLE_NUMBER];

/************************************************************************************************************************/
/****                            如果采用这个onMouse()函数的话，则可以画出鼠标拖动矩形框的4种情形                        ****/
/************************************************************************************************************************/
void onMouse(int event, int x, int y, int, void*)
{
	//Point origin;//不能在这个地方进行定义，因为这是基于消息响应的函数，执行完后origin就释放了，所以达不到效果。
	if (select_flag)
	{
		select.x = MIN(origin.x, x);//不一定要等鼠标弹起才计算矩形框，而应该在鼠标按下开始到弹起这段时间实时计算所选矩形框
		select.y = MIN(origin.y, y);
		select.width = abs(x - origin.x);//算矩形宽度和高度
		select.height = abs(y - origin.y);
		select &= Rect(0, 0, frame.cols, frame.rows);//保证所选矩形框在视频显示区域之内

		//        rectangle(frame,select,Scalar(0,0,255),3,8,0);//显示手动选择的矩形框
	}
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		select_flag = true;//鼠标按下的标志赋真值
		tracking = false;
		select_show = true;
		after_select_frames = 0;//还没开始选择，或者重新开始选择，计数为0
		origin = Point(x, y);//保存下来单击是捕捉到的点
		select = Rect(x, y, 0, 0);//这里一定要初始化，因为在opencv中Rect矩形框类内的点是包含左上角那个点的，但是不含右下角那个点。
	}
	else if (event == CV_EVENT_LBUTTONUP)
	{
		select_flag = false;
		tracking = true;
		select_show = false;
		after_select_frames = 1;//选择完后的那一帧当做第1帧
	}
}

/****粒子权值降序排列函数****/
int particle_decrease(const void *p1, const void *p2)//不可改变粒子
{
	PARTICLE* _p1 = (PARTICLE*)p1;
	PARTICLE* _p2 = (PARTICLE*)p2;
	if (_p1->weight<_p2->weight)
		return 1;
	else if (_p1->weight>_p2->weight)
		return -1;
	return 0;//相等的情况下返回0
}

int main(int argc, unsigned char* argv[])
{
	char c;
	Mat target_img, track_img;
	Mat target_hist, track_hist;
	PARTICLE *pParticle;

	/***打开摄像头****/
	VideoCapture cam(0);
	if (!cam.isOpened())
		return -1;

	/****读取一帧图像****/
	cam >> frame;
	if (frame.empty())
		return -1;

	VideoWriter output_dst("demo.avi", CV_FOURCC('M', 'J', 'P', 'G'), 10, frame.size(), 1);

	/****建立窗口****/
	namedWindow("camera", 1);//显示视频原图像的窗口

	/****捕捉鼠标****/
	setMouseCallback("camera", onMouse, 0);

	while (1)
	{
		/****读取一帧图像****/
		cam >> frame;
		if (frame.empty())
			return -1;

		/****将rgb空间转换为hsv空间****/
		cvtColor(frame, hsv, CV_BGR2HSV);

		if (tracking)
		{

			if (1 == after_select_frames)//选择完目标区域后
			{
				/****计算目标模板的直方图特征****/
				target_img = Mat(hsv, select);//在此之前先定义好target_img,然后这样赋值也行，要学会Mat的这个操作
				calcHist(&target_img, 1, channels, Mat(), target_hist, 3, hist_size, ranges);
				normalize(target_hist, target_hist);

				/****初始化目标粒子****/
				pParticle = particles;//指针初始化指向particles数组
				for (int x = 0; x<PARTICLE_NUMBER; x++)
				{
					pParticle->x = cvRound(select.x + 0.5*select.width);//选定目标矩形框中心为初始粒子窗口中心
					pParticle->y = cvRound(select.y + 0.5*select.height);
					pParticle->orix = pParticle->x;//粒子的原始坐标为选定矩形框(即目标)的中心
					pParticle->oriy = pParticle->y;
					pParticle->prex = pParticle->x;//更新上一次的粒子位置
					pParticle->prey = pParticle->y;
					pParticle->rect = select;
					pParticle->prescale = 1;
					pParticle->scale = 1;
					pParticle->hist = target_hist;
					pParticle->weight = 0;
					pParticle++;
				}
			}
			else if (2 == after_select_frames)//从第二帧开始就可以开始跟踪了
			{
				double sum = 0.0;
				pParticle = particles;
				RNG rng;//随机数产生器

				/****更新粒子结构体的大部分参数****/
				for (int i = 0; i<PARTICLE_NUMBER; i++)
				{
					int x, y;
					int xpre, ypre;
					double s, pres;

					xpre = pParticle->x;
					ypre = pParticle->y;
					pres = pParticle->scale;

					/****更新粒子的矩形区域即粒子中心****/
					x = cvRound(A1*(pParticle->x - pParticle->orix) + A2*(pParticle->prex - pParticle->orix) +
						B0*rng.gaussian(sigmax) + pParticle->orix);
					pParticle->x = max(0, min(x, frame.cols - 1));

					y = cvRound(A1*(pParticle->y - pParticle->oriy) + A2*(pParticle->prey - pParticle->oriy) +
						B0*rng.gaussian(sigmay) + pParticle->oriy);
					pParticle->y = max(0, min(y, frame.rows - 1));

					s = A1*(pParticle->scale - 1) + A2*(pParticle->prescale - 1) + B0*(rng.gaussian(sigmas)) + 1.0;
					pParticle->scale = max(1.0, min(s, 3.0));

					pParticle->prex = xpre;
					pParticle->prey = ypre;
					pParticle->prescale = pres;
					//        pParticle->orix=pParticle->orix;
					//        pParticle->oriy=pParticle->oriy;

					//注意在c语言中，x-1.0，如果x是int型，则这句语法有错误,但如果前面加了cvRound(x-0.5)则是正确的
					pParticle->rect.x = max(0, min(cvRound(pParticle->x - 0.5*pParticle->scale*pParticle->rect.width), frame.cols));
					pParticle->rect.y = max(0, min(cvRound(pParticle->y - 0.5*pParticle->scale*pParticle->rect.height), frame.rows));
					pParticle->rect.width = min(cvRound(pParticle->rect.width), frame.cols - pParticle->rect.x);
					pParticle->rect.height = min(cvRound(pParticle->rect.height), frame.rows - pParticle->rect.y);
					//        pParticle->rect.width=min(cvRound(pParticle->scale*pParticle->rect.width),frame.cols-pParticle->rect.x);
					//        pParticle->rect.height=min(cvRound(pParticle->scale*pParticle->rect.height),frame.rows-pParticle->rect.y);

					/****计算粒子区域的新的直方图特征****/
					track_img = Mat(hsv, pParticle->rect);
					calcHist(&track_img, 1, channels, Mat(), track_hist, 3, hist_size, ranges);
					normalize(track_hist, track_hist);

					/****更新粒子的权值****/
					//            pParticle->weight=compareHist(target_hist,track_hist,CV_COMP_INTERSECT);
					//采用巴氏系数计算相似度,永远与最开始的那一目标帧相比较
					pParticle->weight = 1.0 - compareHist(target_hist, track_hist, CV_COMP_BHATTACHARYYA);
					/****累加粒子权值****/
					sum += pParticle->weight;
					pParticle++;
				}

				/****归一化粒子权重****/
				pParticle = particles;
				for (int i = 0; i<PARTICLE_NUMBER; i++)
				{
					pParticle->weight /= sum;
					pParticle++;
				}

				/****根据粒子的权值降序排列****/
				pParticle = particles;
				qsort(pParticle, PARTICLE_NUMBER, sizeof(PARTICLE), &particle_decrease);

				/****根据粒子权重重采样粒子****/
				PARTICLE newParticle[PARTICLE_NUMBER];
				int np = 0, k = 0;
				for (int i = 0; i<PARTICLE_NUMBER; i++)
				{
					np = cvRound(pParticle->weight*PARTICLE_NUMBER);
					for (int j = 0; j<np; j++)
					{
						newParticle[k++] = particles[i];
						if (k == PARTICLE_NUMBER)
							goto EXITOUT;
					}
				}
				while (k<PARTICLE_NUMBER)
					newParticle[k++] = particles[0];
			EXITOUT:
				for (int i = 0; i<PARTICLE_NUMBER; i++)
					particles[i] = newParticle[i];
			}//end else

			//????????这个排序很慢，粒子数一多就卡
			//    qsort(pParticle,PARTICLE_NUMBER,sizeof(PARTICLE),&particle_decrease);

			/****计算粒子期望，采用所有粒子位置的期望值做为跟踪结果****/
			/*Rect_<double> rectTrackingTemp(0.0,0.0,0.0,0.0);
			pParticle=particles;
			for(int i=0;i<PARTICLE_NUMBER;i++)
			{
			rectTrackingTemp.x+=pParticle->rect.x*pParticle->weight;
			rectTrackingTemp.y+=pParticle->rect.y*pParticle->weight;
			rectTrackingTemp.width+=pParticle->rect.width*pParticle->weight;
			rectTrackingTemp.height+=pParticle->rect.height*pParticle->weight;
			pParticle++;
			}*/


			/****计算最大权重目标的期望位置，作为跟踪结果****/
			Rect rectTrackingTemp(0, 0, 0, 0);
			pParticle = particles;
			rectTrackingTemp.x = pParticle->x - 0.5*pParticle->rect.width;
			rectTrackingTemp.y = pParticle->y - 0.5*pParticle->rect.height;
			rectTrackingTemp.width = pParticle->rect.width;
			rectTrackingTemp.height = pParticle->rect.height;



			/****计算最大权重目标的期望位置，采用权值最大的1/4个粒子数作为跟踪结果****/
			/*Rect rectTrackingTemp(0,0,0,0);
			double weight_temp=0.0;
			pParticle=particles;
			for(int i=0;i<PARTICLE_NUMBER/4;i++)
			{
			weight_temp+=pParticle->weight;
			pParticle++;
			}
			pParticle=particles;
			for(int i=0;i<PARTICLE_NUMBER/4;i++)
			{
			pParticle->weight/=weight_temp;
			pParticle++;
			}
			pParticle=particles;
			for(int i=0;i<PARTICLE_NUMBER/4;i++)
			{
			rectTrackingTemp.x+=pParticle->rect.x*pParticle->weight;
			rectTrackingTemp.y+=pParticle->rect.y*pParticle->weight;
			rectTrackingTemp.width+=pParticle->rect.width*pParticle->weight;
			rectTrackingTemp.height+=pParticle->rect.height*pParticle->weight;
			pParticle++;
			}*/


			/****计算最大权重目标的期望位置，采用所有粒子数作为跟踪结果****/
			/*Rect rectTrackingTemp(0,0,0,0);
			pParticle=particles;
			for(int i=0;i<PARTICLE_NUMBER;i++)
			{
			rectTrackingTemp.x+=cvRound(pParticle->rect.x*pParticle->weight);
			rectTrackingTemp.y+=cvRound(pParticle->rect.y*pParticle->weight);
			pParticle++;
			}
			pParticle=particles;
			rectTrackingTemp.width = pParticle->rect.width;
			rectTrackingTemp.height = pParticle->rect.height;*/


			//创建目标矩形区域
			Rect tracking_rect(rectTrackingTemp);

			pParticle = particles;

			/****显示各粒子运动结果****/
			for (int m = 0; m<PARTICLE_NUMBER; m++)
			{
				rectangle(frame, pParticle->rect, Scalar(255, 0, 0), 1, 8, 0);
				pParticle++;
			}

			/****显示跟踪结果****/
			rectangle(frame, tracking_rect, Scalar(0, 0, 255), 3, 8, 0);

			after_select_frames++;//总循环每循环一次，计数加1
			if (after_select_frames>2)//防止跟踪太长，after_select_frames计数溢出
				after_select_frames = 2;
		}

		if (select_show)
			rectangle(frame, select, Scalar(0, 0, 255), 3, 8, 0);//显示手动选择的矩形框
		output_dst << frame;
		//显示视频图片到窗口
		imshow("camera", frame);

		//    select.zeros();
		//键盘响应
		c = (char)waitKey(20);
		if (27 == c)//ESC键
			return -1;
	}

	return 0;
}