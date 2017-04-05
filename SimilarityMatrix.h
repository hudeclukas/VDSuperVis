#pragma once
#include <opencv2/opencv.hpp>
class SimilarityMatrix {
	cv::Mat m_matrix;
	std::vector<float> m_meanSimilarity;
public:
	SimilarityMatrix();
	~SimilarityMatrix();

	template<typename Type>
	cv::Mat& getSimilarityMatrix(std::map<int, Type> items);

	std::vector<float> getMeanSimilarity() { return m_meanSimilarity; }

	cv::Mat getLastSimilarityMatrix() { return m_matrix; }
};

template <typename Type>
cv::Mat& SimilarityMatrix::getSimilarityMatrix(std::map<int, Type> items) {
	m_matrix.create(items.size(), items.size(), CV_64FC1);
	for (auto row = 0; row < m_matrix.rows; ++row)
	{
		m_meanSimilarity.push_back(0);
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
			m_meanSimilarity[row] += similarity;
		}
		m_meanSimilarity[row] /= m_matrix.cols;
	}
	return m_matrix;
}
