#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include<vector>
#include <iostream>
using namespace cv;
using namespace std;

//RGB->HSV->HS_feature
void featureDistract(Mat &source, Mat& feature);//vector<float>& feature);
bool computeBhattacharyyaLikilyhoodHSV(const cv::Mat & feature1, const cv::Mat & feature2, float &likilyhood);
void featureDistrackREH(const Mat &source, Mat& feature);
void featureDistrackEOH(const Mat &source, Mat& feature);
int picelToRange(int &x,int &numRange);//映射像素到直方图子区间