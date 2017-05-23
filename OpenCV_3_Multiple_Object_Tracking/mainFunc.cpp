#include "..//assignment/HungarianAlg.h"//匹配
#include"..//other//function.h"//生成矩阵
#include "mainFunc.h"
#include<iterator>
//#include <boost/asio.hpp>

float dist_thres = 24;//特征距离阈值
int maximum_allowed_skipped_frames = 5;//最大丢失帧数
void filerResizePeopleRect(std::vector<cv::Rect>& peopleRects)
{
	size_t i, j;
	std::vector<cv::Rect> found_filtered;
	for (i = 0; i < peopleRects.size(); i++)
	{
		cv::Rect r = peopleRects[i];
		for (j = 0; j < peopleRects.size(); j++)
			if (j != i && (r & peopleRects[j]) == r)
				break;
		if (j == peopleRects.size())
			found_filtered.push_back(r);
	}
	for (i = 0; i < found_filtered.size(); i++)
	{
		cv::Rect & r = found_filtered[i];
		// the HOG detector returns slightly larger rectangles than the real objects.
		// so we slightly shrink the rectangles to get a nicer output.
		r.x += cvRound(r.width*0.1);
		r.width = cvRound(r.width*0.8);
		r.y += cvRound(r.height*0.07);
		r.height = cvRound(r.height*0.8);
	}
	peopleRects.assign(found_filtered.begin(), found_filtered.end());
}

//请考虑两个集合为空的情况
void matchCurrentFrameBlobsToExistingBlobs(std::vector<Blob> &existingBlobs, std::vector<Blob> &currentFrameBlobs) 
{
	//对匹配成功的目标更新滤波参数
	//预测目标位置
	
	//lambad表达式的运用如下
	//for_each(existingBlobs.begin(), existingBlobs.end(), [](Blob &existingBlob){existingBlob.blnCurrentMatchFoundOrNewBlob = false; existingBlob.predictNextPosition(); });
	for (auto &existingBlob : existingBlobs) 
	{//此处为引用类型，否则只是对副本操作

		existingBlob.blnCurrentMatchFoundOrNewBlob = false;//全部更新为未匹配目标,不是未跟踪目标
		existingBlob.predictNextPosition();//函数内部会调用卡尔曼滤波
	}
	//generate matrix
	vector<float> disMatrixIn;//行跟踪，列检测，按列存储有可能的匹配
	
	assignments_t assignment;//save result of match,大小为行数
	generateDistMatrix(existingBlobs, currentFrameBlobs, disMatrixIn);
	int N = existingBlobs.size();
	int M = currentFrameBlobs.size(); 
	AssignmentProblemSolver assignmentSolver;
	assignmentSolver.Solve(disMatrixIn, N, M, assignment,AssignmentProblemSolver::optimal);
	
	//update status,这一步值更新assignment和已匹配目标，未匹配目标留待下一步解决
	for (int i = 0; i < assignment.size(); i++)
	{
		if (assignment[i] != -1)//存在匹配项
		{
			if (disMatrixIn[i + assignment[i] * N] > dist_thres)//距离太大
			{
				assignment[i] = -1;//更新为未匹配目标,匹配的目标丢失帧数均为0
			//	existingBlobs[i].intNumOfConsecutiveFramesWithoutAMatch = 1;//目标丢失帧数
			}
			else//匹配目标,更新目标
			{
				addBlobToExistingBlobs(currentFrameBlobs[assignment[i]], existingBlobs, i/*待更新目标的下标*/);//距离够小则匹配目标然后跟新该目标参数
				cv::Point currentLocation = existingBlobs[i].centerPositions.back();
				existingBlobs[i].measurement.at<float>(0) = (float)currentLocation.x;
				existingBlobs[i].measurement.at<float>(1) = (float)currentLocation.y;
				existingBlobs[i].KF.correct(existingBlobs[i].measurement);
			}
		}
// 		existingBlobs[i].intNumOfConsecutiveFramesWithoutAMatch++;
	}
	// -----------------------------------
	// If track didn't get detects long time, remove it.
	// -----------------------------------
	for (int i = 0; i < static_cast<int>(existingBlobs.size()); i++)
	{
		if (!existingBlobs[i].blnCurrentMatchFoundOrNewBlob)//当前帧未匹配
		{
			existingBlobs[i].intNumOfConsecutiveFramesWithoutAMatch++;
		}

		if (existingBlobs[i].intNumOfConsecutiveFramesWithoutAMatch > maximum_allowed_skipped_frames)//目标彻底丢失则删除，其实 blnStillBeingTracked没用了
		{
			existingBlobs.erase(existingBlobs.begin() + i);//删掉之后size减小
			assignment.erase(assignment.begin() + i);
			i--;
		}
		else if (existingBlobs[i].intNumOfConsecutiveFramesWithoutAMatch>0)//目标未丢失，只是当前帧未匹配，需要更新标志位
		{
			existingBlobs[i].centerPositions.push_back(existingBlobs[i].predictedNextPosition);//更新当前中心位置，
			existingBlobs[i].updateBoundingRect(existingBlobs[i].predictedNextPosition);//use prediction location to update rectangle.
		}
	}
	// -----------------------------------
	// Search for unassigned detects and start new tracks for them.
	// -----------------------------------
	for (size_t i = 0; i < currentFrameBlobs.size(); ++i)
	{
		if (find(assignment.begin(), assignment.end(), i) == assignment.end())//未匹配则为新目标。
		{
			//今后会在此处添加目标识别的代码，确认是不是数据库里面的目标，是才添加为新的跟踪目标
			currentFrameBlobs[i].ID = Blob::publicID++;//分配ID
			addNewBlob(currentFrameBlobs[i],existingBlobs);
		}
	}

// 	for (auto &currentFrameBlob : currentFrameBlobs) 
// 	{
// 
// 		int intIndexOfLeastDistance = 0;
// 		double dblLeastDistance = 100000.0;
// 
// 		for (unsigned int i = 0; i < existingBlobs.size(); i++)//find the shortest distance between current and existed
// 		{
// 			if (existingBlobs[i].blnStillBeingTracked == true)
// 			{//计算目标外接矩形中心点之间的距离。
// 				double dblDistance = distanceBetweenPoints(currentFrameBlob.centerPositions.back(), existingBlobs[i].predictedNextPosition);
// 
// 				if (dblDistance < dblLeastDistance)
// 				{
// 					dblLeastDistance = dblDistance;//最短距离
// 					intIndexOfLeastDistance = i;//最短距离索引
// 				}
// 			}
// 		}
// 		//以距离判断新旧目标不合适
// 		if (dblLeastDistance < currentFrameBlob.dblCurrentDiagonalSize * 1.15) 
// 		{
// 			//伪添加，实则是更新目标特征,同时更新滤波器特征
// 			addBlobToExistingBlobs(currentFrameBlob, existingBlobs, intIndexOfLeastDistance/*待更新目标的下标*/);//距离够小则匹配目标然后跟新该目标参数
// 			cv::Point currentLocation = existingBlobs[intIndexOfLeastDistance].centerPositions.back();
// 			existingBlobs[intIndexOfLeastDistance].measurement.at<float>(0) = (float)currentLocation.x;
// 			existingBlobs[intIndexOfLeastDistance].measurement.at<float>(1) = (float)currentLocation.y;
// 			existingBlobs[intIndexOfLeastDistance].KF.correct(existingBlobs[intIndexOfLeastDistance].measurement);
// 		}
// 		else 
// 		{
// 			//真正的添加新目标，新目标的滤波参数不用更新
// 			addNewBlob(currentFrameBlob, existingBlobs);//add new target
// 		}
// 
// 	}
// 	//连续5帧丢失才丢失，但是并没有连续5帧匹配成功才跟踪，因为无论如何你也要跟踪才能判断是否匹配成功。
// 	for (auto &existingBlob : existingBlobs) 
// 	{
// 
// 		if (existingBlob.blnCurrentMatchFoundOrNewBlob == false) 
// 		{//未匹配的目标
// 			existingBlob.intNumOfConsecutiveFramesWithoutAMatch++;
// 		}
// 		if (existingBlob.intNumOfConsecutiveFramesWithoutAMatch >= 2) 
// 		{//连续5帧没有匹配目标则丢弃跟踪
// 			existingBlob.blnStillBeingTracked = false;
// 		}
// 
// 	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void addBlobToExistingBlobs(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs, int &intIndex)
{
	//加入匹配目标，相当于更新当前目标的特征参数，全部为当前帧的参数。
	existingBlobs[intIndex].currentContour = currentFrameBlob.currentContour;
	existingBlobs[intIndex].currentBoundingRect = currentFrameBlob.currentBoundingRect;

	existingBlobs[intIndex].centerPositions.push_back(currentFrameBlob.centerPositions.back());

	existingBlobs[intIndex].dblCurrentDiagonalSize = currentFrameBlob.dblCurrentDiagonalSize;
	existingBlobs[intIndex].dblCurrentAspectRatio = currentFrameBlob.dblCurrentAspectRatio;

	existingBlobs[intIndex].blnStillBeingTracked = true;
	existingBlobs[intIndex].blnCurrentMatchFoundOrNewBlob = true;
	existingBlobs[intIndex].intNumOfConsecutiveFramesWithoutAMatch = 0;
	existingBlobs[intIndex].bolbFeature = currentFrameBlob.bolbFeature;//update feature of hsv
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void addNewBlob(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs) 
{

	currentFrameBlob.blnCurrentMatchFoundOrNewBlob = true;
	existingBlobs.push_back(currentFrameBlob);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double distanceBetweenPoints(cv::Point point1, cv::Point point2) 
{

	int intX = abs(point1.x - point2.x);
	int intY = abs(point1.y - point2.y);

	return(sqrt(pow(intX, 2) + pow(intY, 2)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawAndShowContours(cv::Size imageSize, std::vector<std::vector<cv::Point> > contours, std::string strImageName) 
{
	cv::Mat image(imageSize, CV_8UC3, SCALAR_BLACK);

	cv::drawContours(image, contours, -1, SCALAR_WHITE, -1);

	cv::imshow(strImageName, image);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawAndShowContours(cv::Size imageSize, std::vector<Blob> blobs, std::string strImageName)
{

	cv::Mat image(imageSize, CV_8UC3, SCALAR_BLACK);

	std::vector<std::vector<cv::Point> > contours;
	//for_each(blobs.begin(), blobs.end(), [&contours](Blob &blob){if (blob.blnStillBeingTracked == true) {contours.push_back(blob.currentContour);}});
	for (auto &blob : blobs) 
	{
		if (blob.blnStillBeingTracked == true) 
		{
			contours.push_back(blob.currentContour);
		}
	}

	cv::drawContours(image, contours, -1, SCALAR_WHITE, -1);

	cv::imshow(strImageName, image);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawBlobInfoOnImage(std::vector<Blob> &blobs, cv::Mat &imgFrame2Copy) 
{

	for (unsigned int i = 0; i < blobs.size(); i++) 
	{

		//if (blobs[i].blnStillBeingTracked == true)
		{
			cv::rectangle(imgFrame2Copy, blobs[i].currentBoundingRect, SCALAR_RED, 2);

			int intFontFace = CV_FONT_HERSHEY_SIMPLEX;
			double dblFontScale = blobs[i].dblCurrentDiagonalSize / 60.0;
			int intFontThickness = (int)std::round(dblFontScale * 1.0);

			cv::putText(imgFrame2Copy, std::to_string(blobs[i].ID), blobs[i].centerPositions.back(), intFontFace, dblFontScale, SCALAR_GREEN, intFontThickness);
		}
	}
}








