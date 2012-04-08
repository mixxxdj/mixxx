#ifndef WAVEFORMWIDGETFACTORY_H
#define WAVEFORMWIDGETFACTORY_H

#include <singleton.h>
#include "configobject.h"

#include "waveform/widgets/waveformwidgettype.h"

#include <QObject>

#include <vector>

class WWaveformViewer;
class WaveformWidgetAbstract;
class ControlObjectThreadMain;
class QTimer;
class QTime;

class WaveformWidgetAbstractHandle {
public:
    WaveformWidgetAbstractHandle();

    WaveformWidgetType::Type getType() const { return m_type;}
    QString getDisplayName() const { return m_displayString;}
    bool isActive() const { return m_active;}

private:
    bool m_active;
    WaveformWidgetType::Type m_type;
    QString m_displayString;

    friend class WaveformWidgetFactory;
};

class WaveformWidgetHolder {
private:
    WaveformWidgetHolder( WaveformWidgetAbstract* waveformWidget,
                          WWaveformViewer* waveformViewer,
                          const QDomNode& visualNodeCache);

    WaveformWidgetAbstract* m_waveformWidget;
    WWaveformViewer* m_waveformViewer;

    //NOTE: (vRince) used to be able to recreat wavfeom widget without
    //re-building entire skin ... but it does not work for the moment
    QDomNode m_visualNodeCache;

    friend class WaveformWidgetFactory;
};

//########################################

class WaveformWidgetFactory : public QObject, public Singleton<WaveformWidgetFactory> {
    Q_OBJECT

public:
    bool setConfig(ConfigObject<ConfigValue>* config);

    //creates the waveform widget and bind it to the viewer
    //clean-up every thing if needed
    bool setWaveformWidget(WWaveformViewer* viewer, const QDomElement &node);

    void setFrameRate( int frameRate);
    int getFrameRate() const { return m_frameRate;}
    double getActualFrameRate() const { return m_actualFrameRate;}

    bool isOpenGLAvailable() const { return m_openGLAvailable;}
    QString getOpenGLVersion() const { return m_openGLVersion;}

    bool isOpenGlShaderAvailable() const { return m_openGLShaderAvailable;}

    bool setWidgetType( int handleIndex);
    WaveformWidgetType::Type getType() const { return m_type;}

    void setDefaultZoom(int zoom);
    int getDefaultZoom() const { return m_defaultZoom;}

    void setZoomSync(bool sync);
    int isZoomSync() const { return m_zoomSync;}

    const std::vector<WaveformWidgetAbstractHandle> getAvailableTypes() const { return m_waveformWidgetHandles;}
    void destroyWidgets();

    void addTimerListener(QWidget* pWidget);

public slots:
    void start();
    void stop();

    void notifyZoomChange(WWaveformViewer *viewer);

signals:
    void waveformUpdateTick();

protected:
    void timerEvent(QTimerEvent *timerEvent);

protected:
    WaveformWidgetFactory();
    virtual ~WaveformWidgetFactory();

    friend class Singleton<WaveformWidgetFactory>;

private slots:
    void refresh();

private:
    void evaluateWidgets();
    WaveformWidgetAbstract* createWaveformWidget( WaveformWidgetType::Type type, WWaveformViewer* viewer);
    int findIndexOf( WWaveformViewer* viewer) const;

private:
    //All type of available widgets
    std::vector<WaveformWidgetAbstractHandle> m_waveformWidgetHandles;

    //Currently in use widgets/visual/node
    std::vector<WaveformWidgetHolder> m_waveformWidgetHolders;

    WaveformWidgetType::Type m_type;

    ConfigObject<ConfigValue>* m_config;

    //bool m_skipRender;
    int m_frameRate;
    int m_mainTimerId;
    int m_defaultZoom;
    bool m_zoomSync;

    bool m_openGLAvailable;
    QString m_openGLVersion;
    bool m_openGLShaderAvailable;

    //Debug
    QTime* m_time;
    int m_lastFrameTime;
    double m_actualFrameRate;
};

#endif // WAVEFORMWIDGETFACTORY_H
