#include<conio.h>           // it may be necessary to change or remove this line if not using Windows
#include<opencv.hpp>
#include<iostream>

#include "Blob.h"
#include "mainFunc.h"
#include "../featureDistract/feature.h"
#include "../vibesource/vibeAll.h"

#define SHOW_STEPS            // un-comment or comment this line to show steps or not

typedef ViBeSequential<1, Manhattan<1> >  ViBeclass;
cv::VideoCapture capVideo;
std::vector<Blob> blobs;//the object been tracking
std::vector<Blob> currentFrameBlobs;

int main(void) 
{
    cv::Mat imgFrame1;//current frame
	vector<float> targetFeature;//coloue feature
	capVideo.open("1.avi");
	int frameCount = 0;

	//vibe have got
	boost::int32_t height = capVideo.get(CV_CAP_PROP_FRAME_HEIGHT);
	boost::int32_t width = capVideo.get(CV_CAP_PROP_FRAME_WIDTH);
	ViBeclass* vibe = nullptr;
	cv::Mat currentFrameGray(height, width, CV_8UC1);

    if (!capVideo.isOpened()) {                                                 // if unable to open video file
        std::cout << "error reading video file" << std::endl << std::endl;      // show error message
        _getch();                    // it may be necessary to change or remove this line if not using Windows
        return(0);                                                              // and exit program
    }
    if (capVideo.get(CV_CAP_PROP_FRAME_COUNT) < 2) {
        std::cout << "error: video file must have at least two frames";
        _getch();
        return(0);
    }
    char chCheckForEscKey = 0;
	cv::Mat segmentationMap(height, width, CV_8UC1);
	
    while (capVideo.isOpened() && chCheckForEscKey != 27) 
	{
		capVideo.read(imgFrame1);
// 		if (!imgFrame1.isContinuous())//存储是否连续，否则退出
// 		{
// 			return  0;
// 		}
		cv::cvtColor(imgFrame1, currentFrameGray, CV_RGB2GRAY);
		frameCount++;
		cout << "当前帧：" << frameCount << endl;//......................................................................
		if (frameCount <= 30)
		{
			/* Instantiation of ViBe. */
			if (frameCount==1)
			{
				vibe = new ViBeclass(height, width, currentFrameGray.data);
			}
			vibe->segmentation(currentFrameGray.data, segmentationMap.data);
			vibe->update(currentFrameGray.data, segmentationMap.data);
			medianBlur(segmentationMap, segmentationMap, 3);
			imshow("segmentation", segmentationMap);
			waitKey(1);
			continue;
		}

		//featureDistract(imgFrame1, targetFeature);//特征提取
		
		vibe->segmentation(currentFrameGray.data, segmentationMap.data);
		vibe->update(currentFrameGray.data, segmentationMap.data);
		medianBlur(segmentationMap, segmentationMap, 3);
		imshow("segmentation", segmentationMap);
        //......................................
		cv::Mat segmentationMapCopy = segmentationMap.clone();//used to findcountour which could change source image
        std::vector<std::vector<cv::Point> > contours;
		cv::findContours(segmentationMapCopy, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		
		//drawAndShowContours(segmentationMap.size(), contours, "imgContours");//显示帧原始检测结果

		if (contours.empty())
		{
			continue;
		}
		//vector<std::vector<cv::Point>> convexHulls(contours.size());//temporary value.
		std::vector<cv::Point> tempConvexHull;
		cv::Rect tempRect;
		
        for (unsigned int i = 0; i < contours.size(); i++) 
		{
			//第一步筛选：轮廓点数量
			if (contours[i].size()<=10) continue;
			//第二步筛选：凸包的面积
            cv::convexHull(contours[i], tempConvexHull); //compute 凸包
			double  temp=cv::contourArea(tempConvexHull);
			if (temp<=40) continue;
			//rectangle(imgFrame1, tempRect.tl(), tempRect.br(), cv::Scalar(rand() % 255, rand() % 255, rand() % 255), 3);
				
            //drawAndShowContours(segmentationMap.size(), convexHulls, "imgConvexHulls");  
			//记得以后将新的blob存储在堆存储区，记得自己回收
            Blob possibleBlob(tempConvexHull);//initialize a target
            if (possibleBlob.currentBoundingRect.area() > 100 &&
                possibleBlob.dblCurrentAspectRatio >= 0.2 &&
                possibleBlob.dblCurrentAspectRatio <= 1.25 &&
                possibleBlob.currentBoundingRect.width > 20 &&
                possibleBlob.currentBoundingRect.height > 40 &&
                possibleBlob.dblCurrentDiagonalSize > 30.0 &&
                (/*cv::contourArea(possibleBlob.currentContour)*/temp / (double)possibleBlob.currentBoundingRect.area()) > 0.40) 
			{
				//at feature distract
				featureDistract(imgFrame1(possibleBlob.currentBoundingRect),possibleBlob.bolbFeature);
                currentFrameBlobs.push_back(possibleBlob);
            }
        }

		drawAndShowContours(segmentationMap.size(), currentFrameBlobs, "imgCurrentFrameBlobs");//show current detected targets
		cout << "跟踪目标数：" << blobs.size() << "                检测目标数：" << currentFrameBlobs.size() << endl;
        if (frameCount==31) 
		{
            for (auto &currentFrameBlob : currentFrameBlobs) 
			{
				currentFrameBlob.ID = Blob::publicID++;//分配ID
                blobs.push_back(currentFrameBlob);
            }
        }
        else 
		{
            matchCurrentFrameBlobsToExistingBlobs(blobs, currentFrameBlobs);
        }

		drawAndShowContours(segmentationMap.size(), blobs, "imgBlobs");//show tracked targets

        Mat imgFrame2Copy = imgFrame1.clone();          // get another copy of frame 2 since we changed the previous frame 2 copy in the processing above

        drawBlobInfoOnImage(blobs, imgFrame2Copy);

        cv::imshow("imgFrame2Copy", imgFrame2Copy);

        //cv::waitKey(0);                 // uncomment this line to go frame by frame for debugging

        currentFrameBlobs.clear();
        chCheckForEscKey = cv::waitKey(1);
		if(chCheckForEscKey == ' ')
		{										    // if the user did not press esc (i.e. we reached the end of the video)
			chCheckForEscKey = cv::waitKey(0);                         // hold the windows open to allow the "end of video" message to show
		}
    }
	delete vibe;
	destroyAllWindows();
    return 0;
}
