#include "glwaveformwidget.h"

GLWaveformWidget::GLWaveformWidget( const char* group, QWidget *parent) :
    WaveformWidgetRenderer( group),
    QGLWidget(parent)
{
}

GLWaveformWidget::~GLWaveformWidget()
{

}

void GLWaveformWidget::initializeGL()
{

}

void GLWaveformWidget::paintGL()
{

}
