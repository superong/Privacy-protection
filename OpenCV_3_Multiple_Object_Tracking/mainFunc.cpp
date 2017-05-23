#include "..//assignment/HungarianAlg.h"//ƥ��
#include"..//other//function.h"//���ɾ���
#include "mainFunc.h"
#include<iterator>
//#include <boost/asio.hpp>

float dist_thres = 24;//����������ֵ
int maximum_allowed_skipped_frames = 5;//���ʧ֡��
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

//�뿼����������Ϊ�յ����
void matchCurrentFrameBlobsToExistingBlobs(std::vector<Blob> &existingBlobs, std::vector<Blob> &currentFrameBlobs) 
{
	//��ƥ��ɹ���Ŀ������˲�����
	//Ԥ��Ŀ��λ��
	
	//lambad���ʽ����������
	//for_each(existingBlobs.begin(), existingBlobs.end(), [](Blob &existingBlob){existingBlob.blnCurrentMatchFoundOrNewBlob = false; existingBlob.predictNextPosition(); });
	for (auto &existingBlob : existingBlobs) 
	{//�˴�Ϊ�������ͣ�����ֻ�ǶԸ�������

		existingBlob.blnCurrentMatchFoundOrNewBlob = false;//ȫ������Ϊδƥ��Ŀ��,����δ����Ŀ��
		existingBlob.predictNextPosition();//�����ڲ�����ÿ������˲�
	}
	//generate matrix
	vector<float> disMatrixIn;//�и��٣��м�⣬���д洢�п��ܵ�ƥ��
	
	assignments_t assignment;//save result of match,��СΪ����
	generateDistMatrix(existingBlobs, currentFrameBlobs, disMatrixIn);
	int N = existingBlobs.size();
	int M = currentFrameBlobs.size(); 
	AssignmentProblemSolver assignmentSolver;
	assignmentSolver.Solve(disMatrixIn, N, M, assignment,AssignmentProblemSolver::optimal);
	
	//update status,��һ��ֵ����assignment����ƥ��Ŀ�꣬δƥ��Ŀ��������һ�����
	for (int i = 0; i < assignment.size(); i++)
	{
		if (assignment[i] != -1)//����ƥ����
		{
			if (disMatrixIn[i + assignment[i] * N] > dist_thres)//����̫��
			{
				assignment[i] = -1;//����Ϊδƥ��Ŀ��,ƥ���Ŀ�궪ʧ֡����Ϊ0
			//	existingBlobs[i].intNumOfConsecutiveFramesWithoutAMatch = 1;//Ŀ�궪ʧ֡��
			}
			else//ƥ��Ŀ��,����Ŀ��
			{
				addBlobToExistingBlobs(currentFrameBlobs[assignment[i]], existingBlobs, i/*������Ŀ����±�*/);//���빻С��ƥ��Ŀ��Ȼ����¸�Ŀ�����
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
		if (!existingBlobs[i].blnCurrentMatchFoundOrNewBlob)//��ǰ֡δƥ��
		{
			existingBlobs[i].intNumOfConsecutiveFramesWithoutAMatch++;
		}

		if (existingBlobs[i].intNumOfConsecutiveFramesWithoutAMatch > maximum_allowed_skipped_frames)//Ŀ�곹�׶�ʧ��ɾ������ʵ blnStillBeingTrackedû����
		{
			existingBlobs.erase(existingBlobs.begin() + i);//ɾ��֮��size��С
			assignment.erase(assignment.begin() + i);
			i--;
		}
		else if (existingBlobs[i].intNumOfConsecutiveFramesWithoutAMatch>0)//Ŀ��δ��ʧ��ֻ�ǵ�ǰ֡δƥ�䣬��Ҫ���±�־λ
		{
			existingBlobs[i].centerPositions.push_back(existingBlobs[i].predictedNextPosition);//���µ�ǰ����λ�ã�
			existingBlobs[i].updateBoundingRect(existingBlobs[i].predictedNextPosition);//use prediction location to update rectangle.
		}
	}
	// -----------------------------------
	// Search for unassigned detects and start new tracks for them.
	// -----------------------------------
	for (size_t i = 0; i < currentFrameBlobs.size(); ++i)
	{
		if (find(assignment.begin(), assignment.end(), i) == assignment.end())//δƥ����Ϊ��Ŀ�ꡣ
		{
			//�����ڴ˴����Ŀ��ʶ��Ĵ��룬ȷ���ǲ������ݿ������Ŀ�꣬�ǲ����Ϊ�µĸ���Ŀ��
			currentFrameBlobs[i].ID = Blob::publicID++;//����ID
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
// 			{//����Ŀ����Ӿ������ĵ�֮��ľ��롣
// 				double dblDistance = distanceBetweenPoints(currentFrameBlob.centerPositions.back(), existingBlobs[i].predictedNextPosition);
// 
// 				if (dblDistance < dblLeastDistance)
// 				{
// 					dblLeastDistance = dblDistance;//��̾���
// 					intIndexOfLeastDistance = i;//��̾�������
// 				}
// 			}
// 		}
// 		//�Ծ����ж��¾�Ŀ�겻����
// 		if (dblLeastDistance < currentFrameBlob.dblCurrentDiagonalSize * 1.15) 
// 		{
// 			//α��ӣ�ʵ���Ǹ���Ŀ������,ͬʱ�����˲�������
// 			addBlobToExistingBlobs(currentFrameBlob, existingBlobs, intIndexOfLeastDistance/*������Ŀ����±�*/);//���빻С��ƥ��Ŀ��Ȼ����¸�Ŀ�����
// 			cv::Point currentLocation = existingBlobs[intIndexOfLeastDistance].centerPositions.back();
// 			existingBlobs[intIndexOfLeastDistance].measurement.at<float>(0) = (float)currentLocation.x;
// 			existingBlobs[intIndexOfLeastDistance].measurement.at<float>(1) = (float)currentLocation.y;
// 			existingBlobs[intIndexOfLeastDistance].KF.correct(existingBlobs[intIndexOfLeastDistance].measurement);
// 		}
// 		else 
// 		{
// 			//�����������Ŀ�꣬��Ŀ����˲��������ø���
// 			addNewBlob(currentFrameBlob, existingBlobs);//add new target
// 		}
// 
// 	}
// 	//����5֡��ʧ�Ŷ�ʧ�����ǲ�û������5֡ƥ��ɹ��Ÿ��٣���Ϊ���������ҲҪ���ٲ����ж��Ƿ�ƥ��ɹ���
// 	for (auto &existingBlob : existingBlobs) 
// 	{
// 
// 		if (existingBlob.blnCurrentMatchFoundOrNewBlob == false) 
// 		{//δƥ���Ŀ��
// 			existingBlob.intNumOfConsecutiveFramesWithoutAMatch++;
// 		}
// 		if (existingBlob.intNumOfConsecutiveFramesWithoutAMatch >= 2) 
// 		{//����5֡û��ƥ��Ŀ����������
// 			existingBlob.blnStillBeingTracked = false;
// 		}
// 
// 	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void addBlobToExistingBlobs(Blob &currentFrameBlob, std::vector<Blob> &existingBlobs, int &intIndex)
{
	//����ƥ��Ŀ�꣬�൱�ڸ��µ�ǰĿ�������������ȫ��Ϊ��ǰ֡�Ĳ�����
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








