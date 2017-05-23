#include "feature.h"
void featureDistract(Mat &source, Mat& feature)//RGB->HSV->HS_feature
{
	Mat hsv;
	//feature.clear();
	cvtColor(source, hsv, CV_BGR2HSV);
	int hbin = 20, sbin = 32;
	int histsize[] = { hbin, sbin };
	int chanels[] = { 0, 1 };//支取两个通道
	float hrange[] = { 0, 180 };
	float srange[] = { 0, 255 };
	const float *range[] = { hrange, srange };
	//Mat hist;
	calcHist(&hsv,1, chanels, Mat(), feature,2, histsize, range);
	normalize(feature, feature, 0, 1, NORM_MINMAX,-1,Mat());//归一化到0~255之间
	/**************显示直方图********************/
// 	int scale = 6;
// 	double maxVal = .0;
// 	minMaxLoc(feature, 0, &maxVal, 0, 0);//用最大值进行缩放
// 	Mat histImg = Mat::zeros(sbin*scale, hbin*scale, CV_8UC3);
// 	for (int h = 0; h < hbin; h++)
// 	{
// 		for (int s = 0; s < sbin; s++)
// 			{
// 	 			float binVal = feature.at<float>(h, s);
// 				//feature.push_back(binVal);
// 	 			int intensity = cvRound(binVal*0.9 * 255 / maxVal);
// 				rectangle(histImg, Point(h*scale, s*scale), Point((h + 1)*scale - 1, (s + 1)*scale - 1), Scalar::all(intensity), CV_FILLED);
// 	 		}
// 	 }
// 	namedWindow("H-S Histogram");
// 	imshow("H-S Histogram", histImg);
	/**************显示直方图********************/

// 	for (int h = 0; h < hbin;h++)
// 	{
// 		for (int s = 0; s < sbin;s++)
// 		{
// 			feature.push_back(hist.at<float>(h, s));
// 			//cout << feature[h*sbin+s] << ' ';
// 		}
// 		//cout << endl;
// 	}
}

//巴氏距离越小越相似，0则完全相同。
bool computeBhattacharyyaLikilyhoodHSV(const cv::Mat & feature1, const cv::Mat & feature2, float &likilyhood)
{
// 	float Lhsv = static_cast<float>(compareHist(feature1,feature2,CV_COMP_BHATTACHARYYA));
// 	likilyhood = static_cast<float>(exp(-0.5*Lhsv));//lamuda=0.5,越大越相似
	likilyhood = static_cast<float>(exp(compareHist(feature1, feature2, CV_COMP_BHATTACHARYYA)));
	return true;
}

int picelToRange(int &x, int &sizeRange)//映射像素到直方图子区间,第二个参数为子区间大小。
{
	return x/sizeRange;
}

void featureDistrackEOH(const Mat &source, Mat& feature);


void featureDistrackREH(const Mat &source, Mat& feature)//区域边缘统计直方图,source为RGB颜色空间图像
{
	//range 0~255
	int numRange = 32;//单通道区间数
	int sizeRange = 256 / numRange;//子区间大小
	Mat hsvImg;
	//hsvImg.create(source.size(), source.type());
	cvtColor(source, hsvImg, CV_BGR2HSV);
	int nChannels = hsvImg.channels();
	int nCols = hsvImg.cols;
	int nRows = hsvImg.rows*nChannels;
	if (hsvImg.isContinuous())
	{
		nCols *= nRows;
		nRows = 1;
	}
	int i, j;
	uchar* p;
	for (i = 0; i < nRows; ++i)
	{
		p = hsvImg.ptr<uchar>(i);
		for (j = 0; j < nCols; ++j)
		{
			p[j] = p[j]/sizeRange;
		}
	}	
	
	vector<Mat> mv;
	split(hsvImg, mv);

}
