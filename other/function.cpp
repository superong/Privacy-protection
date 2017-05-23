#include "function.h"
#include "..//featureDistract/feature.h"//������ȡHSV
#include "..//OpenCV_3_Multiple_Object_Tracking//mainFunc.h"//distanceBetweenPoints()
float MAX_Value = 65535;
bool RectOverlap(const Rect& box1, const Rect& box2, float& value)
{
	if (box1.x > box2.x + box2.width) { value = 0; return true; }
	if (box1.y > box2.y + box2.height){ value = 0; return true; }
	if (box1.x + box1.width < box2.x) { value = 0; return true; }
	if (box1.y + box1.height < box2.y){ value = 0; return true; }
	int colInt = min(box1.x + box1.width, box2.x + box2.width) - max(box1.x, box2.x);//�ص�����
	int rowInt = min(box1.y + box1.height, box2.y + box2.height) - max(box1.y, box2.y);//�ص�����
	int intersection = colInt * rowInt;//�ص����
	int area1 = box1.width*box1.height;
	int area2 = box2.width*box2.height;
	value = static_cast<float>(intersection) / static_cast<float>(area1 + area2 - intersection);
	return true;
}

bool distManhattan(const Blob &target1, const Blob & target2, float& distValue)//����Ŀ���������������پ���
{
	distValue=static_cast<float>(std::abs(target1.centerPositions.back().x - target2.centerPositions.back().x)
		+ std::abs(target1.centerPositions.back().y - target2.centerPositions.back().y)); 

	//manhattanDistance = 0.25*(target1.currentBoundingRect.height + target1.currentBoundingRect.width);
	return true;
}


bool gateFunction(const Blob& track, const Blob& measure)//�ź���������ɸѡ
{
	float thresholdOverlap = 0.4, thresholdManhattan;//�ص������������پ���

	float overlap, distmanhattan;
	thresholdManhattan = static_cast<float>( 0.3*(track.currentBoundingRect.height + track.currentBoundingRect.width
		+ measure.currentBoundingRect.height + measure.currentBoundingRect.width));//�����پ�����ֵ:sigma=0.3

	RectOverlap(track.currentBoundingRect, measure.currentBoundingRect, overlap);
	distManhattan(track, measure, distmanhattan);
	//�ص�����̫С�������پ���̫��
	if (overlap<thresholdOverlap || distmanhattan>thresholdManhattan)
	{
		return false;
	}
	else
		return true;
}

bool generateDistMatrix(const vector<Blob>& tracks, const vector<Blob>& measures, vector< vector<float> > &distMatrix)//����������
{
	distMatrix = vector<vector<float>>(tracks.size(), vector<float>(measures.size(),MAX_Value));//�и��٣��м��,��ʼ��Ϊ���޴����
	float likilyhood, dist_euclid;
	float a = static_cast<float>(0.8);
	for (size_t row = 0; row < tracks.size();row++)
	{
		for (size_t col = 0; col < measures.size();col++)
		{
			if (gateFunction(tracks[row],measures[col]))//����ͨ���ź������ټ���������,δͨ�����򱣳ֳ�ʼֵ
			{
				computeBhattacharyyaLikilyhoodHSV(tracks[row].bolbFeature, measures[col].bolbFeature, likilyhood);//���Ͼ���ԽСԽ���ƣ�0����ȫ��ͬ
				dist_euclid = static_cast<float>(distanceBetweenPoints(tracks[row].centerPositions.back(), measures[col].centerPositions.back()));
				std::cout << "likilyhood:" << likilyhood << "    dist_euclid:" << dist_euclid << std::endl;
				distMatrix[row][col] = a*likilyhood + (1 - a)*dist_euclid;//ŷ�Ͼ���+�������ƶ�,a���ڸ������Ȩ��
			}
		}
	}
	return true;
}

bool generateDistMatrix(const vector<Blob>& tracks, const vector<Blob>& measures, vector<float> &distMatrix)
	//vector<int>&lostTargets, vector<int>& newTargets, vector<int>&stay_tracks, vector<int>&stay_measures)//����������:һά����,�����벻���ܵ�ƥ�䡣
{
	distMatrix = vector<float>(tracks.size()*measures.size(), MAX_Value);//�и��٣��м��,��ʼ��Ϊ���޴����,��ʱ����

	float likilyhood, dist_euclid;
	int N = tracks.size();
	float a = static_cast<float>(0.7);
	for (size_t row = 0; row < tracks.size(); row++)
	{
		for (size_t col = 0; col < measures.size(); col++)
		{
			if (gateFunction(tracks[row], measures[col]))//����ͨ���ź������ټ���������,δͨ�����򱣳ֳ�ʼֵ
			{
				computeBhattacharyyaLikilyhoodHSV(tracks[row].bolbFeature, measures[col].bolbFeature, likilyhood);
				dist_euclid = static_cast<float>(distanceBetweenPoints(tracks[row].centerPositions.back(), measures[col].centerPositions.back()));
				std::cout << "likilyhood:" << likilyhood << "    dist_euclid:" << dist_euclid << std::endl;
				distMatrix[row+col*N] = a*likilyhood + (1 - a)*dist_euclid;//ŷ�Ͼ���+�������ƶ�
			}
		}
	}
	return true;
}
