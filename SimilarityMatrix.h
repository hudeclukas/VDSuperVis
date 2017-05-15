#pragma once
#include <opencv2/opencv.hpp>

class Superpixel;
class SimilarityMatrix {
	cv::Mat m_matrix;
	std::map<int, double> m_meanSimilarity;
public:
	SimilarityMatrix();
	~SimilarityMatrix();

	cv::Mat& getSimilarityMatrix(std::map<int, Superpixel> items);
	std::map<int, double> getSimilarity(int idx);

	std::map<int, double> getMeanSimilarity() { return m_meanSimilarity; }

	cv::Mat getLastSimilarityMatrix() { return m_matrix; }
};