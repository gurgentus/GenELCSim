#include "part.h"
#include <iostream>

Part::Part()
{
    //m_modelToWorld = m_program->uniformLocation("modelToWorld");
}

Part::~Part()
{
//    wingMesh.cleanup();
//    aileronMesh.cleanup();
}



void Part::initializeControls()
{
//    aileronAngleControl = new Control(controlLayout, drawingWidget);
//    aileronAngleControl->setValue(20);
    SBody::initializeControls();
//    sizeControl->setValue(4);
//    aileron.sizeControl = sizeControl;
}
void Part::updateControls()
{
    SBody::updateControls();
//    aileron.position = position;
//    aileron.toPosRotMatrix = toPosRotMatrix;

//    if (aileronAngleControl != nullptr) // && (aileronAngleControl->m_value != aileronAngleControl->old_value))
//    {
//        aileron.translate(0.1, 0, 0);
//        aileron.rotate(aileronAngleControl->m_value, 0,1,0);
//        aileron.translate(-0.1, 0, 0);
//    }

}

bool Part::setupDefaultMesh()
{

}
