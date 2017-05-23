#pragma once
#include <opencv.hpp>
#include "..//OpenCV_3_Multiple_Object_Tracking\Blob.h"

//#include "..//assignment//HungarianAlg.h"//KM�㷨���������㷨
using namespace cv;
//extern float MAX_Value = 65535;
bool RectOverlap(const Rect& box1, const Rect& box2,float& value);//������ص�����
bool distManhattan(const Blob &target1, const Blob & target2, int& distValue, int manhattanDistance);//����Ŀ���������������پ���
bool gateFunction(const Blob& track, const Blob& measure);//�ź���������ɸѡ
bool generateDistMatrix(const vector<Blob>& tracks, const vector<Blob>& measures, vector< vector<float> > &distMatrix);//���������󣺶�ά����
bool generateDistMatrix(const vector<Blob>& tracks, const vector<Blob>& measures, vector<float> &distMatrix);// , vector<int>&lostTargets, vector<int>& newTargets, vector<int>&stay_tracks, vector<int>&stay_measures)//����������:һά����,�����벻���ܵ�ƥ�䡣
