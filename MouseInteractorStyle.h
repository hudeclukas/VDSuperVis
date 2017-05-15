#ifndef MOUSEINTERACTORSTYLE_H
#define MOUSEINTERACTORSTYLE_H

#include <vtkSmartPointer.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkOutlineSource.h>

#include <QObject>
#include <QtWidgets/QColorDialog>

#include <set>

class vtkActor;
class vtkActorCollection;

class Superpixel;

#pragma once

// Handle mouse events
class MouseInteractorStyle : public QObject, public vtkInteractorStyleTrackballCamera {
	Q_OBJECT
public:
	static MouseInteractorStyle* New();

	MouseInteractorStyle() {
		PickNextColor[2] = 1.0;
	}


	vtkTypeMacro(MouseInteractorStyle, vtkInteractorStyleTrackballCamera);

	virtual void OnLeftButtonDown() override;
	virtual void OnMouseWheelForward() override;
	virtual void OnMouseWheelBackward() override;

	virtual void HighlightProp(vtkProp *prop);
	virtual void HighlightNextProps();
	virtual void ReHighlightProps();

	void setActors(std::map<int, Superpixel> *actors);
	std::set<int> getAllSelectedActorIds();

	public slots:
	void setSelect(bool select);
	void setMultiSelect(bool multi);
	void setFirstSelectColor(QColor color);
	QColor getFirstSelectColor();
	void setNextSelectColor(QColor color);
	QColor getNextSelectColor();
	
	void clearSelection();

signals:
	void pickedActor(int idx);

private:

	// for picking actors
								   bool		select = false;
								   bool		multiSelect = false;
								 QColor		firstSelectColor;
								 QColor		nextSelectColor;
								 double		PickNextColor[3];
									int		selectedActorId = -1;
			  vtkSmartPointer<vtkActor>		selectedActor = nullptr;
		 std::map<vtkIdType, vtkActor*>		nextSelectedActors;
		 std::map<vtkIdType, vtkActor*>		outlineSelectedActors;
  std::map<vtkIdType,vtkOutlineSource*>		OutlineNext;
			  std::map<int, Superpixel>		*actors = nullptr;
};

#endif