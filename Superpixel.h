#pragma once
#include <string>
#include <vector>
#include <map>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPolyData.h>

class Superpixel {
	
	vtkSmartPointer<vtkPoints> m_points;
	vtkSmartPointer<vtkUnsignedCharArray> m_colors;

public:
	int m_id;
	double red, green, blue, depth, nX, nY, nZ;


	Superpixel();
	virtual ~Superpixel();
	static std::map<int, Superpixel> loadSuperpixels(std::string image, std::string indexes, std::string features);

	vtkSmartPointer<vtkPolyData> getSuperpixel();

};
