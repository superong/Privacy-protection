/* Copyright - Benjamin Laugraud <blaugraud@ulg.ac.be> - 2016
 * Copyright - Marc Van Droogenbroeck <m.vandroogenbroeck@ulg.ac.be> - 2016
 *
 * ViBe is covered by a patent (see http://www.telecom.ulg.ac.be/research/vibe).
 *
 * Permission to use ViBe without payment of fee is granted for nonprofit
 * educational and research purposes only.
 *
 * This work may not be copied or reproduced in whole or in part for any
 * purpose.
 *
 * Copying, reproduction, or republishing for any purpose shall require a
 * license. Please contact the authors in such cases. All the code is provided
 * without any guarantee.
 *
 * This simple example program takes a path to a video sequence as an argument.
 * When it is executed, two windows are opened: one displaying the input
 * sequence, and one displaying the segmentation maps produced by ViBe. Note
 * that this program uses the a polychromatic version of ViBe with 3 channels.
 */
#include <cstddef>
#include <ctime>
#include <iostream>


#include <cv.h>
#include <highgui.h>

//#include <opencv2/opencv.hpp>


#include "vibesource/ViBe.h"
#include "vibesource/Manhattan.h"
#include "vibesource/types.h"

using namespace std;
using namespace cv;
using namespace ViBe;

int main(int argc, char** argv) {
//   if (argc != 2) {
//     cerr << "A video file must be given as an argument to the program!";
//     cerr << endl;
// 
//     return EXIT_FAILURE;
//   }

  /* Parameterization of ViBe. */
  typedef ViBeSequential<3, Manhattan<3> >  ViBeClass;
  char ch='0';
  /* Random seed. */
  srand(time(NULL));

  cv::VideoCapture decoder("1.avi");
  cv::Mat frame;

  ViBe::int32_t height = decoder.get(CV_CAP_PROP_FRAME_HEIGHT);
  ViBe::int32_t width = decoder.get(CV_CAP_PROP_FRAME_WIDTH);

  ViBeClass* vibe = NULL;
  cv::Mat segmentationMap(height, width, CV_8UC1);
  bool firstFrame = true;

  while (decoder.read(frame) && ch != 27) {
    if (firstFrame) {
      /* Instantiation of ViBe. */
		vibe = new ViBeClass(height, width, frame.data);
      firstFrame = false;
    }

    /* Segmentation and update. */
    vibe->segmentation(frame.data, segmentationMap.data);
    vibe->update(frame.data, segmentationMap.data);

    /* Post-processing: 3x3 median filter. */
    medianBlur(segmentationMap, segmentationMap, 3);

    imshow("Input video", frame);
    imshow("Segmentation by ViBe", segmentationMap);
	ch = cvWaitKey(1);
    if (ch==' ')
    {
		cv::waitKey(0);
    }
    
  }

  delete vibe;

  cvDestroyAllWindows();
  decoder.release();

  return EXIT_SUCCESS;
}
