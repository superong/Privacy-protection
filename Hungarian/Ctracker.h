#pragma once
#include "Kalman.h"
#include "HungarianAlg.h"
#include "defines.h"
#include <iostream>
#include <vector>
#include <memory>
#include <array>

// --------------------------------------------------------------------------
//包含滤波器和跟踪目标细节信息
class CTrack
{
public:
	CTrack(const Point_t& p, const cv::Rect& rect, track_t dt, track_t Accel_noise_mag, size_t trackID)
		:
		track_id(trackID),
		skipped_frames(0),
		prediction(p),
		lastRect(rect),
		KF(p, dt, Accel_noise_mag)
	{
	}
	//计算点距离
	track_t CalcDist(const Point_t& p)
	{
		Point_t diff = prediction - p;
		return sqrtf(diff.x * diff.x + diff.y * diff.y);
	}
	//计算矩形间隔（外形+中心点）
	track_t CalcDist(const cv::Rect& r)
	{
		cv::Rect rect = GetLastRect();

		std::array<track_t, 4> diff;
		diff[0] = prediction.x - lastRect.width / 2 - r.x;
		diff[1] = prediction.y - lastRect.height / 2 - r.y;
		diff[2] = static_cast<track_t>(lastRect.width - r.width);
		diff[3] = static_cast<track_t>(lastRect.height - r.height);

		track_t dist = 0;
		for (size_t i = 0; i < diff.size(); ++i)
		{
			dist += diff[i] * diff[i];
		}
		return sqrtf(dist);
	}

	//预测目标点，更新滤波器，更新轨迹
	void Update(const Point_t& p, const cv::Rect& rect, bool dataCorrect, size_t max_trace_length)
	{
		KF.GetPrediction();//结果可返回，也可保存于成员变量lastResult
		prediction = KF.Update(p, dataCorrect);//更新之后立刻做出预测

		if (dataCorrect)
		{
			lastRect = rect;
		}

		if (trace.size() > max_trace_length)//轨迹控制在最大长度内
		{
			trace.erase(trace.begin(), trace.end() - max_trace_length);//先进先出
		}

		trace.push_back(prediction);//然后长度会比最大长度大一
	}

	std::vector<Point_t> trace;//目标轨迹，point vector
	size_t track_id;//目标ID
	size_t skipped_frames; 

	//由上一个跟踪目标的预测中心得到预测矩形框
	cv::Rect GetLastRect()
	{
		return cv::Rect(
			static_cast<int>(prediction.x - lastRect.width / 2),
			static_cast<int>(prediction.y - lastRect.height / 2),
			lastRect.width,
			lastRect.height);
	}

private:
	Point_t prediction;//预测目标中心
	cv::Rect lastRect;//预测目标矩形框，由预测中心得到
	TKalmanFilter KF;
};

// --------------------------------------------------------------------------
//只有update这个关键函数
class CTracker
{
public:
	CTracker(track_t dt_, track_t Accel_noise_mag_, track_t dist_thres_ = 60, size_t maximum_allowed_skipped_frames_ = 10, size_t max_trace_length_ = 10);
	~CTracker(void);

	enum DistType
	{
		CentersDist = 0,
		RectsDist = 1
	};

	std::vector<std::unique_ptr<CTrack>> tracks;//跟踪目标智能指针向量
	void Update(const std::vector<Point_t>& detections, const std::vector<cv::Rect>& rects, DistType distType);

private:
	track_t dt;
	track_t Accel_noise_mag;
	track_t dist_thres;//距离阈值
    size_t maximum_allowed_skipped_frames;//最大丢失帧数
    size_t max_trace_length;//轨迹最大长度
	size_t NextTrackID;//下一个跟踪目标ID
}; 
