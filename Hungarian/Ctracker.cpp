#include "Ctracker.h"

// ---------------------------------------------------------------------------
// Tracker. Manage tracks. Create, remove, update.
// ---------------------------------------------------------------------------
CTracker::CTracker(
	track_t dt_,
	track_t Accel_noise_mag_,
	track_t dist_thres_,
	size_t maximum_allowed_skipped_frames_,
	size_t max_trace_length_
	)
	:
	dt(dt_),
	Accel_noise_mag(Accel_noise_mag_),
	dist_thres(dist_thres_),
	maximum_allowed_skipped_frames(maximum_allowed_skipped_frames_),
	max_trace_length(max_trace_length_),
	NextTrackID(0)
{
}
// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
//只有update这个关键函数
void CTracker::Update(
	const std::vector<Point_t>& detections,
	const std::vector<cv::Rect>& rects,
	DistType distType
	)
{
	assert(detections.size() == rects.size());//检测目标中心点和矩形框必须数量相等

	// -----------------------------------
	// If there is no tracks yet, then every cv::Point begins its own track.
	// -----------------------------------
	if (tracks.size() == 0)//已跟踪目标向量为空，直接将检测目标作为跟踪目标
	{
		// If no tracks yet
		for (size_t i = 0; i < detections.size(); ++i)
		{
			tracks.push_back(std::make_unique<CTrack>(detections[i], rects[i], dt, Accel_noise_mag, NextTrackID++));
			//tracks.emplace_back(detections[i], rects[i], dt, Accel_noise_mag, NextTrackID++);
		}
	}

	size_t N = tracks.size();		// 已跟踪目标集合
	size_t M = detections.size();	// 当前帧检测目标集合

	assignments_t assignment; // 匹配结果向量

	if (!tracks.empty())
	{
		distMatrix_t Cost(N * M);//构建距离向量，以一维矩阵代替二维矩阵，模跟踪目标数量N取整为检测目标索引，取余为跟踪目标索引，每一个检测目标与其他所有跟踪目标的距离是连续存储的

		switch (distType)
		{
		case CentersDist:   //中心距离
			for (size_t i = 0; i < tracks.size(); i++)
			{
				for (size_t j = 0; j < detections.size(); j++)
				{
					Cost[i + j * N] = tracks[i]->CalcDist(detections[j]);
				}
			}
			break;

		case RectsDist:   //矩形距离
			for (size_t i = 0; i < tracks.size(); i++)
			{
				for (size_t j = 0; j < detections.size(); j++)
				{
					Cost[i + j * N] = tracks[i]->CalcDist(rects[j]);
				}
			}
			break;
		}

		// -----------------------------------
		// Solving assignment problem (tracks and predictions of Kalman filter)
		// -----------------------------------
		//之后再细看
		AssignmentProblemSolver APS;
		APS.Solve(Cost, N, M, assignment, AssignmentProblemSolver::optimal);

		// -----------------------------------
		// clean assignment from pairs with large distance
		// -----------------------------------
		for (size_t i = 0; i < assignment.size(); i++)
		{
			if (assignment[i] != -1)
			{
				if (Cost[i + assignment[i] * N] > dist_thres)
				{
					assignment[i] = -1;
					tracks[i]->skipped_frames = 1;
				}
			}
			else
			{
				// If track have no assigned detect, then increment skipped frames counter.
				tracks[i]->skipped_frames++;
			}
		}

		// -----------------------------------
		// If track didn't get detects long time, remove it.
		// -----------------------------------
		for (int i = 0; i < static_cast<int>(tracks.size()); i++)//连续多帧丢失跟踪目标则移除
		{
			if (tracks[i]->skipped_frames > maximum_allowed_skipped_frames)
			{
				tracks.erase(tracks.begin() + i);
				assignment.erase(assignment.begin() + i);
				i--;
			}
		}
	}

	// -----------------------------------
	// Search for unassigned detects and start new tracks for them.
	// -----------------------------------
	for (size_t i = 0; i < detections.size(); ++i)
	{
		if (find(assignment.begin(), assignment.end(), i) == assignment.end())
		{
			tracks.push_back(std::make_unique<CTrack>(detections[i], rects[i], dt, Accel_noise_mag, NextTrackID++));
			//tracks.emplace_back(detections[i], rects[i], dt, Accel_noise_mag, NextTrackID++);
		}
	}

	// Update Kalman Filters state

	for (size_t i = 0; i<assignment.size(); i++)
	{
		// If track updated less than one time, than filter state is not correct.

		if (assignment[i] != -1) // If we have assigned detect, then update using its coordinates,
		{
			tracks[i]->skipped_frames = 0;
			tracks[i]->Update(detections[assignment[i]], rects[assignment[i]], true, max_trace_length);
		}
		else				     // if not continue using predictions
		{
			tracks[i]->Update(Point_t(), cv::Rect(), false, max_trace_length);
		}
	}

}
// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
CTracker::~CTracker(void)
{
}