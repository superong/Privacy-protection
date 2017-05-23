// Blob.h

#ifndef MY_BLOB
#define MY_BLOB

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/tracking.hpp> //kalman filter
///////////////////////////////////////////////////////////////////////////////////////////////////
class Blob {
public:
    std::vector<cv::Point> currentContour;//the countour of tracking-target
    cv::Rect currentBoundingRect;//bounding rectangle
	/*std::vector<float>*/
	cv::Mat bolbFeature;
    std::vector<cv::Point> centerPositions;//the center of bounding rectangle，相当于轨迹。

    double dblCurrentDiagonalSize;//对角线长度？
    double dblCurrentAspectRatio;//宽高比率

    bool blnCurrentMatchFoundOrNewBlob;//是否匹配

    bool blnStillBeingTracked;//teh state of being tracking 

    int intNumOfConsecutiveFramesWithoutAMatch;//连续多少帧没有被成功匹配

    cv::Point predictedNextPosition;//预测位置
   
	cv::KalmanFilter KF;
	cv::Mat measurement;
	static int publicID;//公共ID，用于分配目标ID
	int ID;//目标身份，默认为-1.
public:
	Blob(std::vector<cv::Point> &_contour);
    void predictNextPosition(void);//内部调用卡尔曼滤波
	bool updateBoundingRect(const cv::Point pt);
};

#endif    // MY_BLOB


