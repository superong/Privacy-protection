#include "feature.h"
void featureDistract(Mat &source, Mat& feature)//RGB->HSV->HS_feature
{
	Mat hsv;
	//feature.clear();
	cvtColor(source, hsv, CV_BGR2HSV);
	int hbin = 20, sbin = 32;
	int histsize[] = { hbin, sbin };
	int chanels[] = { 0, 1 };//֧ȡ����ͨ��
	float hrange[] = { 0, 180 };
	float srange[] = { 0, 255 };
	const float *range[] = { hrange, srange };
	//Mat hist;
	calcHist(&hsv,1, chanels, Mat(), feature,2, histsize, range);
	normalize(feature, feature, 0, 1, NORM_MINMAX,-1,Mat());//��һ����0~255֮��
	/**************��ʾֱ��ͼ********************/
// 	int scale = 6;
// 	double maxVal = .0;
// 	minMaxLoc(feature, 0, &maxVal, 0, 0);//�����ֵ��������
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
	/**************��ʾֱ��ͼ********************/

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

//���Ͼ���ԽСԽ���ƣ�0����ȫ��ͬ��
bool computeBhattacharyyaLikilyhoodHSV(const cv::Mat & feature1, const cv::Mat & feature2, float &likilyhood)
{
// 	float Lhsv = static_cast<float>(compareHist(feature1,feature2,CV_COMP_BHATTACHARYYA));
// 	likilyhood = static_cast<float>(exp(-0.5*Lhsv));//lamuda=0.5,Խ��Խ����
	likilyhood = static_cast<float>(exp(compareHist(feature1, feature2, CV_COMP_BHATTACHARYYA)));
	return true;
}

int picelToRange(int &x, int &sizeRange)//ӳ�����ص�ֱ��ͼ������,�ڶ�������Ϊ�������С��
{
	return x/sizeRange;
}

void featureDistrackEOH(const Mat &source, Mat& feature);


void featureDistrackREH(const Mat &source, Mat& feature)//�����Եͳ��ֱ��ͼ,sourceΪRGB��ɫ�ռ�ͼ��
{
	//range 0~255
	int numRange = 32;//��ͨ��������
	int sizeRange = 256 / numRange;//�������С
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
