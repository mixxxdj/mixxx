#ifndef GLWAVEFORMWIDGET_H
#define GLWAVEFORMWIDGET_H

#include <QtOpenGL/QGLWidget>

class WaveformWidgetRenderer;

class GLWaveformWidget : public QGLWidget
{
public:
    GLWaveformWidget( const char* group, QWidget* parent);
    virtual ~GLWaveformWidget();

    WaveformWidgetRenderer* getRenderer() { return m_waveformWidgetRenderer;}

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void paintEvent(QPaintEvent* event);

private:
    WaveformWidgetRenderer* m_waveformWidgetRenderer;

};

#endif // GLWAVEFORMWIDGET_H
