#include "control.h"

#include <iostream>
#include <QTimer>
#include <cmath>
#include "myopenglwidget.h"
#include <string>

Control::~Control()
{
}

// Called on value changed of the slider and updates the simulated object
// and the label text
void Control::setFValue(int value)
{           
    old_value = m_value;
    m_value = value*scale_;
    sbody_->UpdateControls();
    value_label->setText(QString::number(value*scale_));

}

// Returns the current value of the control
double Control::value()
{
    return m_value;
}
