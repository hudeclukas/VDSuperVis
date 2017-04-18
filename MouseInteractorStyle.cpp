#include "MouseInteractorStyle.h"

#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkCellPicker.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkOutlineSource.h>
#include <vtkProperty.h>
#include <vtkOutlineSource.h>
#include <vtkRenderWindow.h>

vtkStandardNewMacro(MouseInteractorStyle);

void MouseInteractorStyle::OnLeftButtonDown()
{
	if (select) {
		int* clickPos = this->Interactor->GetEventPosition();

		// Pick from this location.
		vtkSmartPointer<vtkCellPicker>  picker = vtkSmartPointer<vtkCellPicker>::New();
		picker->SetTolerance(0.005);
		
		picker->Pick(clickPos[0], clickPos[1], 0, this->DefaultRenderer);

		if (picker->GetCellId() != -1) {
			actors->InitTraversal();
			for (vtkIdType i = 0; i < actors->GetNumberOfItems(); i++) {
				vtkSmartPointer<vtkActor> a = actors->GetNextItem();
				if (a == picker->GetActor())
				{
					if (multiSelect)
					{
						if (!selectedActor)
						{
							selectedActor = picker->GetActor();
							this->HighlightProp(selectedActor);
						}
						else if (a != selectedActor)
						{
							nextSelectedActors[i] = picker->GetActor();
							this->HighlightNextProps();
						}
					}
					else {
						selectedActor = picker->GetActor();
						this->HighlightProp(selectedActor);
					}
					emit pickedActor(i);
					break;
				}
			}
		}
	}
	this->DefaultRenderer->GetRenderWindow()->Render();
	// Forward events
	vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}

void MouseInteractorStyle::setActors(vtkActorCollection* actors)
{
	this->actors = actors;
	this->clearSelection();
}

void MouseInteractorStyle::setSelect(bool select)
{
	this->select = select;
}

void MouseInteractorStyle::setMultiSelect(bool multi)
{
	this->multiSelect = multi;
}

void MouseInteractorStyle::setFirstSelectColor(QColor color)
{
	firstSelectColor = color;
	if (selectedActor)
	{
		this->PickColor[0] = color.redF();
		this->PickColor[1] = color.greenF();
		this->PickColor[2] = color.blueF();
		this->HighlightProp(selectedActor);
	}
	this->DefaultRenderer->GetRenderWindow()->Render();
}

QColor MouseInteractorStyle::getFirstSelectColor()
{
	return firstSelectColor;
}

void MouseInteractorStyle::setNextSelectColor(QColor color) {
	nextSelectColor = color;
	if (selectedActor) {
		this->PickColor[0] = color.redF();
		this->PickColor[1] = color.greenF();
		this->PickColor[2] = color.blueF();
		this->HighlightNextProps();
	}
	this->DefaultRenderer->GetRenderWindow()->Render();
}

void MouseInteractorStyle::clearSelection()
{
	for (auto &actor : outlineSelectedActors)
	{
		this->DefaultRenderer->RemoveActor(actor.second);
	}
	//this->DefaultRenderer->RemoveActor(this->OutlineActor);
	//this->OutlineActor = nullptr;
	this->DefaultRenderer->GetRenderWindow()->Render();
	if (selectedActor) 
	{
		selectedActor->Delete();
		selectedActor = nullptr;
	}
	nextSelectedActors.clear();
	outlineSelectedActors.clear();
	OutlineNext.clear();
}

QColor MouseInteractorStyle::getNextSelectColor() {
	return nextSelectColor;
}

void MouseInteractorStyle::HighlightProp(vtkProp* prop) {
	//no prop picked now
	if (!prop) {
		//was there previously?
		if (this->PickedRenderer != nullptr && this->OutlineActor) {
			this->PickedRenderer->RemoveActor(this->OutlineActor);
			this->PickedRenderer = nullptr;
		}
	}
	//prop picked now
	else {
		if (!this->OutlineActor) {
			// have to defer creation to get right type
			this->OutlineActor = vtkActor::New();
			this->OutlineActor->PickableOff();
			this->OutlineActor->DragableOff();
			this->OutlineActor->SetMapper(this->OutlineMapper);
			this->OutlineActor->GetProperty()->SetColor(this->PickColor);
			this->OutlineActor->GetProperty()->SetLineWidth(2.);
			this->OutlineActor->GetProperty()->SetAmbient(1.0);
			this->OutlineActor->GetProperty()->SetDiffuse(0.0);
		}
		else {
			auto colors = this->OutlineActor->GetProperty()->GetColor();
			if (colors[0] != this->PickColor[0] || 
				colors[1] != this->PickColor[1] || 
				colors[2] != this->PickColor[2])
			{
				this->OutlineActor->GetProperty()->SetColor(this->PickColor);
			}
		}
		//check if picked in different renderer to previous pick
		if (this->CurrentRenderer != this->PickedRenderer) {
			if (this->PickedRenderer != nullptr && this->OutlineActor) {
				this->PickedRenderer->RemoveActor(this->OutlineActor);
			}
			if (this->CurrentRenderer != nullptr) {
				this->CurrentRenderer->AddActor(this->OutlineActor);
			}
			else {
				vtkWarningMacro(<< "no current renderer on the interactor style.");
			}
			this->PickedRenderer = this->CurrentRenderer;
		}

		this->Outline->SetBounds(prop->GetBounds());
	}
}

void MouseInteractorStyle::HighlightNextProps()
{
	for (auto actor : nextSelectedActors) {
		// have to defer creation to get right type
		if (!outlineSelectedActors[actor.first]) {
			outlineSelectedActors[actor.first] = vtkActor::New();
			outlineSelectedActors[actor.first]->PickableOff();
			outlineSelectedActors[actor.first]->DragableOff();
			outlineSelectedActors[actor.first]->GetProperty()->SetColor(this->PickNextColor);
			outlineSelectedActors[actor.first]->GetProperty()->SetLineWidth(2.);
			outlineSelectedActors[actor.first]->GetProperty()->SetAmbient(1.0);
			outlineSelectedActors[actor.first]->GetProperty()->SetDiffuse(0.0);
			this->DefaultRenderer->AddActor(outlineSelectedActors[actor.first]);
		}
		OutlineNext[actor.first] = vtkOutlineSource::New();
		OutlineNext[actor.first]->SetBounds(actor.second->GetBounds());
		OutlineNext[actor.first]->GenerateFacesOff();
		vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInputConnection(OutlineNext[actor.first]->GetOutputPort());
		outlineSelectedActors[actor.first]->SetMapper(mapper);
	}
	this->DefaultRenderer->GetRenderWindow()->Render();
}

void MouseInteractorStyle::ReHighlightProps()
{
	this->HighlightProp(selectedActor);
	this->HighlightNextProps();
}
