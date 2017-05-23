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
    std::vector<cv::Point> centerPositions;//the center of bounding rectangle���൱�ڹ켣��

    double dblCurrentDiagonalSize;//�Խ��߳��ȣ�
    double dblCurrentAspectRatio;//��߱���

    bool blnCurrentMatchFoundOrNewBlob;//�Ƿ�ƥ��

    bool blnStillBeingTracked;//teh state of being tracking 

    int intNumOfConsecutiveFramesWithoutAMatch;//��������֡û�б��ɹ�ƥ��

    cv::Point predictedNextPosition;//Ԥ��λ��
   
	cv::KalmanFilter KF;
	cv::Mat measurement;
	static int publicID;//����ID�����ڷ���Ŀ��ID
	int ID;//Ŀ����ݣ�Ĭ��Ϊ-1.
public:
	Blob(std::vector<cv::Point> &_contour);
    void predictNextPosition(void);//�ڲ����ÿ������˲�
	bool updateBoundingRect(const cv::Point pt);
};

#endif    // MY_BLOB


