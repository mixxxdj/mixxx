#ifndef GLWAVEFORMWIDGET_H
#define GLWAVEFORMWIDGET_H

#include <QtOpenGL/QGLWidget>
#include "waveformwidget.h"

class GLWaveformWidget : public QGLWidget, public WaveformWidgetRenderer
{
public:
    GLWaveformWidget( const char* group, QWidget *parent = 0);
    ~GLWaveformWidget();

    void initializeGL();
    void paintGL();

};

#endif // GLWAVEFORMWIDGET_H
