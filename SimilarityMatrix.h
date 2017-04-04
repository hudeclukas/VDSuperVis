#pragma once
#include <opencv2/opencv.hpp>
class SimilarityMatrix {
	cv::Mat m_matrix;
public:
	SimilarityMatrix();
	~SimilarityMatrix();

	template<typename Type>
	cv::Mat& getSimilarityMatrix(std::map<int, Type> items);
	template<typename Type>
	std::vector<float> getDistances(std::map<int, Type> items);
	cv::Mat getLastSimilarityMatrix() { return m_matrix; }
};

template <typename Type>
cv::Mat& SimilarityMatrix::getSimilarityMatrix(std::map<int, Type> items) {
	m_matrix.create(items.size(), items.size(), CV_64FC1);
	for (auto row = 0; row < m_matrix.rows; ++row)
	{
		for (auto col = 0; col < m_matrix.cols; ++col)
		{
			if (col == row) 
			{
				m_matrix.at<double>(row, col) = 1;
				continue;
			}
			Type it1 = items[row];
			Type it2 = items[col];
			double similarity = it1.cosineSimilarity(it2);
			m_matrix.at<double>(row, col) = similarity;
			m_matrix.at<double>(col, row) = similarity;
		}
	}
	return m_matrix;
}

template <typename Type>
std::vector<float> SimilarityMatrix::getDistances(std::map<int, Type> items)
{
	std::vector<float> x(items.size(), 0);
	return x;
}
