#ifndef WAVEFORMWIDGETFACTORY_H
#define WAVEFORMWIDGETFACTORY_H

#include <singleton.h>
#include "configobject.h"

#include "waveform/widgets/waveformwidgettype.h"

#include <QObject>
#include <QVector>

class WWaveformViewer;
class WaveformWidgetAbstract;
class QTimer;
class QTime;

class WaveformWidgetAbstractHandle {
  public:
    WaveformWidgetAbstractHandle() {
        m_active = true;
        m_type = WaveformWidgetType::Count_WaveformwidgetType;
    }

    WaveformWidgetType::Type getType() const { return m_type;}
    QString getDisplayName() const { return m_displayString;}
    bool isActive() const { return m_active;}

  private:
    bool m_active;
    WaveformWidgetType::Type m_type;
    QString m_displayString;
    friend class WaveformWidgetFactory;
};

//########################################

class WaveformWidgetFactory : public QObject, public Singleton<WaveformWidgetFactory> {
    Q_OBJECT

  public:
    bool setConfig(ConfigObject<ConfigValue>* config);

    //creates the waveform widget and bind it to the viewer
    //clean-up every thing if needed
    bool setWaveformWidget( WWaveformViewer* viewer);

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
    void onZoomChange( WaveformWidgetAbstract* widget);

    const QVector<WaveformWidgetAbstractHandle> getAvailableTypes() const { return m_waveformWidgetHandles;}
    void destroyWidgets();

    void addTimerListener(QWidget* pWidget);

  public slots:
    void start();
    void stop();

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

    QVector<WaveformWidgetAbstractHandle> m_waveformWidgetHandles;
    QVector<WaveformWidgetAbstract*> m_waveformWidgets;

    WaveformWidgetType::Type m_type;

    ConfigObject<ConfigValue>* m_config;

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
