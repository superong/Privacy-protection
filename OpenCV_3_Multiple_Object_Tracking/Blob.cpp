// Blob.cpp

#include "Blob.h"
extern cv::VideoCapture capVideo;//used to update boundingRect
///////////////////////////////////////////////////////////////////////////////////////////////////
int Blob::publicID = 0;//静态成员变量要在类外初始化

bool Blob::updateBoundingRect(const cv::Point pt)//根据矩形中心点更新外接矩形,以后再考虑pt超出图片范围的情况
{
	int height = static_cast<int>(capVideo.get(CV_CAP_PROP_FRAME_HEIGHT));
	int width = static_cast<int>(capVideo.get(CV_CAP_PROP_FRAME_WIDTH));
	//cv::Rect currentBoundingRect = currentBoundingRect;//使用原先的矩形框大小
	cv::Point temCenterPoint(currentBoundingRect.br().x + currentBoundingRect.tl().x, currentBoundingRect.br().y + currentBoundingRect.tl().y);
	int deltax = pt.x - temCenterPoint.x;//计算位移
	int deltay = pt.y - temCenterPoint.y;
	currentBoundingRect.x += deltax;
	currentBoundingRect.y += deltay;//计算移动后矩形,高宽不变
// 	if (currentBoundingRect.x < 0)//矩形的定位顶点坐标为左上顶点
// 	{
// 		currentBoundingRect.width += currentBoundingRect.x;
// 		currentBoundingRect.x = 0;
// 	}
// 	else if (currentBoundingRect.br().x > width-1)
// 	{
// 		currentBoundingRect.width += width - 1 - currentBoundingRect.br().x;//因为是从0开始计数
// 	}
// 
// 	if (currentBoundingRect.y < 0)
// 	{
// 		currentBoundingRect.height = +currentBoundingRect.y;
// 		currentBoundingRect.y = 0;
// 	}
// 	else if (currentBoundingRect.y > height-1)
// 	{
// 		currentBoundingRect.height += height - 1 - currentBoundingRect.y;
// 	}
	return true;
}

Blob::Blob(std::vector<cv::Point> & _contour)
{
	cv::RNG rng;
    currentContour = _contour;
	currentBoundingRect = cv::boundingRect(currentContour);
	ID = -1;//初始化身份信息，用于输出。
    cv::Point currentCenter;
    currentCenter.x = (currentBoundingRect.x + currentBoundingRect.x + currentBoundingRect.width) / 2;
    currentCenter.y = (currentBoundingRect.y + currentBoundingRect.y + currentBoundingRect.height) / 2;

    centerPositions.push_back(currentCenter);//用于生成路径
	//用勾股定理求对角线长度
    dblCurrentDiagonalSize = sqrt(pow(currentBoundingRect.width, 2) + pow(currentBoundingRect.height, 2));

    dblCurrentAspectRatio = (float)currentBoundingRect.width / (float)currentBoundingRect.height;

    blnStillBeingTracked = true;
    blnCurrentMatchFoundOrNewBlob = true;

    intNumOfConsecutiveFramesWithoutAMatch = 0;//默认没有跟丢 
	//滤波器初始化
	KF.init(4, 2, 0);//状态向量维度，观测向量维度
	KF.transitionMatrix = *(cv::Mat_<float>(4, 4) << 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1);  //转移矩阵A  
	setIdentity(KF.measurementMatrix);                                             //测量矩阵H  
	setIdentity(KF.processNoiseCov, cv::Scalar::all(1e-5));                            //系统噪声方差矩阵Q  
	setIdentity(KF.measurementNoiseCov, cv::Scalar::all(1e-1));                        //测量噪声方差矩阵R  
	setIdentity(KF.errorCovPost, cv::Scalar::all(1));                                  //后验错误估计协方差矩阵P  
	KF.statePost.at<float>(0) = currentCenter.x;
	KF.statePost.at<float>(1) = currentCenter.y;

	//rng.fill(KF.statePost, cv::RNG::UNIFORM, 0,100/*currentCenter.x, currentCenter.y*/);   //初始状态值x(0)  
	measurement = cv::Mat::zeros(2, 1, CV_32F);                           //初始测量值x'(0)，因为后面要更新这个值，所以必须先定义 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Blob::predictNextPosition(void) 
{
	cv::Mat predictedMat = KF.predict();//4 X 1
	predictedNextPosition = cv::Point(static_cast<int>(predictedMat.at<float>(0)), static_cast<int>(predictedMat.at<float>(1)));//float->int，自动截断
//     int numPositions = (int)centerPositions.size();//the count of position of current target
// 
//     if (numPositions == 1) {
// 
//         predictedNextPosition.x = centerPositions.back().x;
//         predictedNextPosition.y = centerPositions.back().y;
// 
//     }
//     else if (numPositions == 2) {//relate to speed or velocity
// 
//         int deltaX = centerPositions[1].x - centerPositions[0].x;
//         int deltaY = centerPositions[1].y - centerPositions[0].y;
// 
//         predictedNextPosition.x = centerPositions.back().x + deltaX;
//         predictedNextPosition.y = centerPositions.back().y + deltaY;
// 
//     }
//     else if (numPositions == 3) {//relate to weight of time 2:1
// 
//         int sumOfXChanges = ((centerPositions[2].x - centerPositions[1].x) * 2) +
//             ((centerPositions[1].x - centerPositions[0].x) * 1);
// 
//         int deltaX = (int)std::round((float)sumOfXChanges / 3.0);
// 
//         int sumOfYChanges = ((centerPositions[2].y - centerPositions[1].y) * 2) +
//             ((centerPositions[1].y - centerPositions[0].y) * 1);
// 
//         int deltaY = (int)std::round((float)sumOfYChanges / 3.0);
// 
//         predictedNextPosition.x = centerPositions.back().x + deltaX;
//         predictedNextPosition.y = centerPositions.back().y + deltaY;
// 
//     }
//     else if (numPositions == 4) {// 3:2:1
// 
//         int sumOfXChanges = ((centerPositions[3].x - centerPositions[2].x) * 3) +
//             ((centerPositions[2].x - centerPositions[1].x) * 2) +
//             ((centerPositions[1].x - centerPositions[0].x) * 1);
// 
//         int deltaX = (int)std::round((float)sumOfXChanges / 6.0);
// 
//         int sumOfYChanges = ((centerPositions[3].y - centerPositions[2].y) * 3) +
//             ((centerPositions[2].y - centerPositions[1].y) * 2) +
//             ((centerPositions[1].y - centerPositions[0].y) * 1);
// 
//         int deltaY = (int)std::round((float)sumOfYChanges / 6.0);
// 
//         predictedNextPosition.x = centerPositions.back().x + deltaX;
//         predictedNextPosition.y = centerPositions.back().y + deltaY;
// 
//     }
//     else if (numPositions >= 5) {//using latest difference of 5 frame by weight 4:3:2:1
// 
//         int sumOfXChanges = ((centerPositions[numPositions - 1].x - centerPositions[numPositions - 2].x) * 4) +
//             ((centerPositions[numPositions - 2].x - centerPositions[numPositions - 3].x) * 3) +
//             ((centerPositions[numPositions - 3].x - centerPositions[numPositions - 4].x) * 2) +
//             ((centerPositions[numPositions - 4].x - centerPositions[numPositions - 5].x) * 1);
// 
//         int deltaX = (int)std::round((float)sumOfXChanges / 10.0);
// 
//         int sumOfYChanges = ((centerPositions[numPositions - 1].y - centerPositions[numPositions - 2].y) * 4) +
//             ((centerPositions[numPositions - 2].y - centerPositions[numPositions - 3].y) * 3) +
//             ((centerPositions[numPositions - 3].y - centerPositions[numPositions - 4].y) * 2) +
//             ((centerPositions[numPositions - 4].y - centerPositions[numPositions - 5].y) * 1);
// 
//         int deltaY = (int)std::round((float)sumOfYChanges / 10.0);
// 
//         predictedNextPosition.x = centerPositions.back().x + deltaX;
//         predictedNextPosition.y = centerPositions.back().y + deltaY;
// 
//     }
//     else {
//         // should never get here
//     }

}




