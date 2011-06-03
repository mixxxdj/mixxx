#ifndef WAVEFORMWIDGETFACTORY_H
#define WAVEFORMWIDGETFACTORY_H

#include <singleton.h>

#include "waveform/waveformwidgettype.h"

#include <QObject>
#include <QList>

class WWaveformViewer;
class WaveformWidgetAbstract;
class QTimer;

class WaveformWidgetFactory : public QObject, public Singleton<WaveformWidgetFactory>
{
    Q_OBJECT

public:
    static const int s_numberOfWidget = 4;

    //creates the waveform widget and bind it to the viewer
    //clean-up every thing if needed
    bool setWaveformWidget( WWaveformViewer* viewer);

    void setFrameRate( int frameRate);
    bool setWidgetType( WaveformWidgetType::Type type);

    void destroyWidgets();

public slots:
    void start();
    void stop();

protected:
    WaveformWidgetFactory();
    virtual ~WaveformWidgetFactory();

    friend class Singleton<WaveformWidgetFactory>;

private slots:
    void refresh();

private:
    WaveformWidgetAbstract* createWaveformWidget( WaveformWidgetType::Type type, WWaveformViewer* viewer);

private:
    WaveformWidgetAbstract* m_waveformWidgets[s_numberOfWidget];

    WaveformWidgetType::Type m_type;

    int m_frameRate;
    QTimer* m_timer;
};

#endif // WAVEFORMWIDGETFACTORY_H
