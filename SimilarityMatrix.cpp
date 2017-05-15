#include "SimilarityMatrix.h"
#include "Superpixel.h"


SimilarityMatrix::SimilarityMatrix() {
}


SimilarityMatrix::~SimilarityMatrix() {
}


cv::Mat& SimilarityMatrix::getSimilarityMatrix(std::map<int, Superpixel> items) {
	m_matrix.create(items.begin()->second.max_id, items.begin()->second.max_id, CV_64FC1);
	for (auto row = 0; row < m_matrix.rows; ++row) {
		m_meanSimilarity[row] = 0;
		for (auto col = 0; col < m_matrix.cols; ++col) {
			if (col == row) {
				m_matrix.at<double>(row, col) = 1;
				continue;
			}
			auto f1 = items.find(row);
			auto f2 = items.find(col);
			if (f1 != items.end() && f2 != items.end()) {
				auto it1 = items[row];
				auto it2 = items[col];
				double similarity = it1.cosineSimilarity(it2);
				m_matrix.at<double>(row, col) = similarity;
				m_matrix.at<double>(col, row) = similarity;
				m_meanSimilarity[row] += similarity;
			}
		}
		m_meanSimilarity[row] /= items.size();
	}
	return m_matrix;
}

std::map<int, double> SimilarityMatrix::getSimilarity(int idx)
{
	auto simRow = m_matrix.ptr<double>(idx);
	std::map<int, double> simMap;
	for (auto i = 0; i < m_matrix.rows; i++)
	{
		simMap[i] = simRow[i];
	}
	return simMap;
}
