#pragma once
#include <opencv.hpp>
#include "..//OpenCV_3_Multiple_Object_Tracking\Blob.h"

//#include "..//assignment//HungarianAlg.h"//KM算法，匈牙利算法
using namespace cv;
//extern float MAX_Value = 65535;
bool RectOverlap(const Rect& box1, const Rect& box2,float& value);//求矩形重叠比例
bool distManhattan(const Blob &target1, const Blob & target2, int& distValue, int manhattanDistance);//根据目标中心求其曼哈顿距离
bool gateFunction(const Blob& track, const Blob& measure);//门函数，初步筛选
bool generateDistMatrix(const vector<Blob>& tracks, const vector<Blob>& measures, vector< vector<float> > &distMatrix);//计算距离矩阵：二维向量
bool generateDistMatrix(const vector<Blob>& tracks, const vector<Blob>& measures, vector<float> &distMatrix);// , vector<int>&lostTargets, vector<int>& newTargets, vector<int>&stay_tracks, vector<int>&stay_measures)//计算距离矩阵:一维向量,并分离不可能的匹配。
