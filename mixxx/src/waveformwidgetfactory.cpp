#include "waveformwidgetfactory.h"
#include "waveform/waveformwidgetabstract.h"
#include "widget/wwaveformviewer.h"

//WaveformWidgets
#include "waveform/glwaveformwidget.h"

#include <QTimer>
#include <QWidget>

#include <QDebug>

WaveformWidgetFactory::WaveformWidgetFactory()
{
    for( int i = 0; i < s_numberOfWidget; i++)
        m_waveformWidgets[i] = 0;

    m_timer = new QTimer();
    connect(m_timer,SIGNAL(timeout()),this,SLOT(refresh()));

    setFrameRate(33);

    //TODO here place automatic code to determine if OpenGl is available to choose type by default
    m_type = WaveformWidgetType::FilteredOpenGlWaveform;

    //TODO expose this from
    start();
}

WaveformWidgetFactory::~WaveformWidgetFactory()
{
    delete m_timer;
}

void WaveformWidgetFactory::start()
{
    qDebug() << "WaveformWidgetFactory::start";
    m_timer->start();
}

void WaveformWidgetFactory::stop()
{
    m_timer->stop();
}

void WaveformWidgetFactory::destroyWidgets()
{
    for( int i = 0; i < s_numberOfWidget; i++)
    {
        if( m_waveformWidgets[i])
            delete m_waveformWidgets[i];
        m_waveformWidgets[i] = 0;
    }
}

bool WaveformWidgetFactory::setWaveformWidget( WWaveformViewer* viewer)
{
    int index = -1;
    if( viewer->getWaveformWidget())
    {
        //it already have a WaveformeWidget
        for( int i = 0; i < s_numberOfWidget; i++)
        {
            if( m_waveformWidgets[i] == viewer->getWaveformWidget())
            {
                index = i;
                break;
            }
        }
        if( index == -1)
        {
            qDebug() << "WaveformWidgetFactory::setWaveformWidget - "\
                        "viewer already have a waveform widget but it's not found by the factory !";
        }
        delete viewer->getWaveformWidget();
    }

    WaveformWidgetAbstract* waveformWidget = createWaveformWidget(m_type,viewer);
    waveformWidget->castToQWidget();
    viewer->setWaveformWidget( waveformWidget);

    if( index == -1)
    {
        //search for available slot
        for( int i = 0; i < s_numberOfWidget; i++)
        {
            if( m_waveformWidgets[i] == 0)
            {
                index = i;
                break;
            }
        }
    }

    if( index != -1)
    {
        m_waveformWidgets[index] = waveformWidget;
        qDebug() << "WaveformWidgetFactory::setWaveformWidget - waveforme widget added in factory index" << index;
        return true;
    }
    else
    {
        qDebug() << "WaveformWidgetFactory::setWaveformWidget - no more available slot (" << s_numberOfWidget << ")";
        return false;
    }
}

void WaveformWidgetFactory::setFrameRate( int frameRate)
{
    m_frameRate = std::min(60,frameRate);
    m_timer->setInterval((int)(1000.0/(double)m_frameRate));
}

bool WaveformWidgetFactory::setWidgetType( WaveformWidgetType::Type type)
{
    if( type != m_type)
    {
        //TODO vrince
    }
}

void WaveformWidgetFactory::refresh()
{
    QApplication::processEvents();

    for( int i = 0; i < s_numberOfWidget; i++)
        if( m_waveformWidgets[i])
            m_waveformWidgets[i]->refresh();

    QApplication::processEvents();
}

WaveformWidgetAbstract* WaveformWidgetFactory::createWaveformWidget( WaveformWidgetType::Type type, WWaveformViewer* viewer)
{
    switch(type)
    {
    case WaveformWidgetType::SimplePureQtWaveform : return 0; //TODO
    case WaveformWidgetType::SimpleOpenGlWaveform : return 0; //TODO
    case WaveformWidgetType::FilteredPureQtWaveform : return 0; //TODO
    case WaveformWidgetType::FilteredOpenGlWaveform : return new GLWaveformWidget( viewer->getGroup(), viewer);
    default : return 0;
    }
}
