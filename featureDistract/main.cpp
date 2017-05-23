#include "feature.h"

void  main()
{
	Mat source = imread("d://people3.jpg");
	vector<float> feature;
	featureDistract(source, feature);
	waitKey(0);
	//return 0;
}