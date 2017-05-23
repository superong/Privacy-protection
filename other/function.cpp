#include "function.h"
#include "..//featureDistract/feature.h"//特征提取HSV
#include "..//OpenCV_3_Multiple_Object_Tracking//mainFunc.h"//distanceBetweenPoints()
float MAX_Value = 65535;
bool RectOverlap(const Rect& box1, const Rect& box2, float& value)
{
	if (box1.x > box2.x + box2.width) { value = 0; return true; }
	if (box1.y > box2.y + box2.height){ value = 0; return true; }
	if (box1.x + box1.width < box2.x) { value = 0; return true; }
	if (box1.y + box1.height < box2.y){ value = 0; return true; }
	int colInt = min(box1.x + box1.width, box2.x + box2.width) - max(box1.x, box2.x);//重叠列数
	int rowInt = min(box1.y + box1.height, box2.y + box2.height) - max(box1.y, box2.y);//重叠行数
	int intersection = colInt * rowInt;//重叠面积
	int area1 = box1.width*box1.height;
	int area2 = box2.width*box2.height;
	value = static_cast<float>(intersection) / static_cast<float>(area1 + area2 - intersection);
	return true;
}

bool distManhattan(const Blob &target1, const Blob & target2, float& distValue)//根据目标中心求其曼哈顿距离
{
	distValue=static_cast<float>(std::abs(target1.centerPositions.back().x - target2.centerPositions.back().x)
		+ std::abs(target1.centerPositions.back().y - target2.centerPositions.back().y)); 

	//manhattanDistance = 0.25*(target1.currentBoundingRect.height + target1.currentBoundingRect.width);
	return true;
}


bool gateFunction(const Blob& track, const Blob& measure)//门函数，初步筛选
{
	float thresholdOverlap = 0.4, thresholdManhattan;//重叠比例与曼哈顿距离

	float overlap, distmanhattan;
	thresholdManhattan = static_cast<float>( 0.3*(track.currentBoundingRect.height + track.currentBoundingRect.width
		+ measure.currentBoundingRect.height + measure.currentBoundingRect.width));//曼哈顿距离阈值:sigma=0.3

	RectOverlap(track.currentBoundingRect, measure.currentBoundingRect, overlap);
	distManhattan(track, measure, distmanhattan);
	//重叠比例太小或曼哈顿距离太大
	if (overlap<thresholdOverlap || distmanhattan>thresholdManhattan)
	{
		return false;
	}
	else
		return true;
}

bool generateDistMatrix(const vector<Blob>& tracks, const vector<Blob>& measures, vector< vector<float> > &distMatrix)//计算距离矩阵
{
	distMatrix = vector<vector<float>>(tracks.size(), vector<float>(measures.size(),MAX_Value));//行跟踪，列检测,初始化为无限大距离
	float likilyhood, dist_euclid;
	float a = static_cast<float>(0.8);
	for (size_t row = 0; row < tracks.size();row++)
	{
		for (size_t col = 0; col < measures.size();col++)
		{
			if (gateFunction(tracks[row],measures[col]))//必须通过门函数，再计算距离矩阵,未通过的则保持初始值
			{
				computeBhattacharyyaLikilyhoodHSV(tracks[row].bolbFeature, measures[col].bolbFeature, likilyhood);//巴氏距离越小越相似，0则完全相同
				dist_euclid = static_cast<float>(distanceBetweenPoints(tracks[row].centerPositions.back(), measures[col].centerPositions.back()));
				std::cout << "likilyhood:" << likilyhood << "    dist_euclid:" << dist_euclid << std::endl;
				distMatrix[row][col] = a*likilyhood + (1 - a)*dist_euclid;//欧氏距离+特征相似度,a调节各距离的权重
			}
		}
	}
	return true;
}

bool generateDistMatrix(const vector<Blob>& tracks, const vector<Blob>& measures, vector<float> &distMatrix)
	//vector<int>&lostTargets, vector<int>& newTargets, vector<int>&stay_tracks, vector<int>&stay_measures)//计算距离矩阵:一维向量,并分离不可能的匹配。
{
	distMatrix = vector<float>(tracks.size()*measures.size(), MAX_Value);//行跟踪，列检测,初始化为无限大距离,临时变量

	float likilyhood, dist_euclid;
	int N = tracks.size();
	float a = static_cast<float>(0.7);
	for (size_t row = 0; row < tracks.size(); row++)
	{
		for (size_t col = 0; col < measures.size(); col++)
		{
			if (gateFunction(tracks[row], measures[col]))//必须通过门函数，再计算距离矩阵,未通过的则保持初始值
			{
				computeBhattacharyyaLikilyhoodHSV(tracks[row].bolbFeature, measures[col].bolbFeature, likilyhood);
				dist_euclid = static_cast<float>(distanceBetweenPoints(tracks[row].centerPositions.back(), measures[col].centerPositions.back()));
				std::cout << "likilyhood:" << likilyhood << "    dist_euclid:" << dist_euclid << std::endl;
				distMatrix[row+col*N] = a*likilyhood + (1 - a)*dist_euclid;//欧氏距离+特征相似度
			}
		}
	}
	return true;
}
