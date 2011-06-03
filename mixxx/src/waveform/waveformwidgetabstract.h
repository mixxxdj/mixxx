#ifndef WAVEFORMWIDGETABSTRACT_H
#define WAVEFORMWIDGETABSTRACT_H

#include <QString>
#include "waveformwidgettype.h"

//NOTE: vRince
//This class represent objects the waveformwidgetfactory can holds,
//IMPORTANT all WaveformWidgetAbstract MUST inherist QWidget too !!
//I can do it here because QWidget and QGLWidget are both QWidgets
//so they already have a common QWidget base class (ambigous polymorphism)
//This Abstract can be directly a WaveformWidgetRenderer cause it inherist of QObjects

class QDomNode;
class WaveformWidgetRenderer;

class QResizeEvent;
class QPaintEvent;
class QWidget;

class WaveformWidgetAbstract
{
public:
    WaveformWidgetAbstract( const char* group);
    virtual ~WaveformWidgetAbstract();

    void setup( const QDomNode& node);

    //Type is use by the factory to safely up-cast waveform widgets
    virtual WaveformWidgetType::Type getType() const = 0;
    virtual void castToQWidget() = 0;

    //Safely down-cast once (in constructor) to QWidget and stored in m_widget
    bool isValid() const { return m_widget;}

    void hold();
    void release();

    void refresh();
    void resize( int width, int height);

    //Those information enable automatic combobox creation and waveform selection
    virtual QString getWaveformWidgetName() = 0;
    virtual bool useOpenGl() const = 0;

    WaveformWidgetRenderer* getRenderer() { return m_waveformWidgetRenderer;}

protected:
    WaveformWidgetRenderer* m_waveformWidgetRenderer;
    QWidget* m_widget;
};

#endif // WAVEFORMWIDGETABSTRACT_H
