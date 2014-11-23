#ifndef WAVEFORMWIDGETFACTORY_H
#define WAVEFORMWIDGETFACTORY_H

#include <QObject>
#include <QTime>
#include <QVector>

#include "util/singleton.h"
#include "configobject.h"
#include "waveform/widgets/waveformwidgettype.h"
#include "waveform/waveform.h"
#include "skin/skincontext.h"

class WWaveformViewer;
class WaveformWidgetAbstract;
class QTimer;
class VSyncThread;
class MixxxMainWindow;

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
  public:
    WaveformWidgetHolder();
  private:
    WaveformWidgetHolder(WaveformWidgetAbstract* waveformWidget,
                         WWaveformViewer* waveformViewer,
                         const QDomNode& skinNode,
                         const SkinContext& skinContext);

    WaveformWidgetAbstract* m_waveformWidget;
    WWaveformViewer* m_waveformViewer;
    QDomNode m_skinNodeCache;
    SkinContext m_skinContextCache;

    friend class WaveformWidgetFactory;
};

//########################################

class WaveformWidgetFactory : public QObject, public Singleton<WaveformWidgetFactory> {
    Q_OBJECT
  public:
    //TODO merge this enum woth the waveform analyser one
    enum FilterIndex { All = 0, Low = 1, Mid = 2, High = 3, FilterCount = 4};

    bool setConfig(ConfigObject<ConfigValue>* config);

    //creates the waveform widget and bind it to the viewer
    //clean-up every thing if needed
    bool setWaveformWidget(WWaveformViewer* viewer,
                           const QDomElement &node, const SkinContext& context);

    void setFrameRate(int frameRate);
    int getFrameRate() const { return m_frameRate;}
//    bool getVSync() const { return m_vSyncType;}
    void setEndOfTrackWarningTime(int endTime);
    int getEndOfTrackWarningTime() const { return m_endOfTrackWarningTime;}

    bool isOpenGLAvailable() const { return m_openGLAvailable;}
    QString getOpenGLVersion() const { return m_openGLVersion;}

    bool isOpenGlShaderAvailable() const { return m_openGLShaderAvailable;}

    bool setWidgetType(WaveformWidgetType::Type type);
    bool setWidgetTypeFromHandle(int handleIndex);
    WaveformWidgetType::Type getType() const { return m_type;}

    void setDefaultZoom(int zoom);
    int getDefaultZoom() const { return m_defaultZoom;}

    void setZoomSync(bool sync);
    int isZoomSync() const { return m_zoomSync;}

    void setVisualGain(FilterIndex index, double gain);
    double getVisualGain(FilterIndex index) const;

    void setOverviewNormalized(bool normalize);
    int isOverviewNormalized() const { return m_overviewNormalized;}

    const QVector<WaveformWidgetAbstractHandle> getAvailableTypes() const { return m_waveformWidgetHandles;}
    void getAvailableVSyncTypes(QList<QPair<int, QString > >* list);
    void destroyWidgets();

    void addTimerListener(QWidget* pWidget);

    void startVSync(MixxxMainWindow* mixxxApp);
    void setVSyncType(int vsType);
    int getVSyncType();

    void notifyZoomChange(WWaveformViewer *viewer);

    WaveformWidgetType::Type autoChooseWidgetType() const;

  signals:
    void waveformUpdateTick();
    void waveformMeasured(float frameRate, int droppedFrames);

  protected:
    WaveformWidgetFactory();
    virtual ~WaveformWidgetFactory();

    friend class Singleton<WaveformWidgetFactory>;

  private slots:
    void render();
    void swap();

  private:
    void evaluateWidgets();
    WaveformWidgetAbstract* createWaveformWidget(WaveformWidgetType::Type type, WWaveformViewer* viewer);
    int findIndexOf(WWaveformViewer* viewer) const;

    //All type of available widgets

    QVector<WaveformWidgetAbstractHandle> m_waveformWidgetHandles;

    //Currently in use widgets/visual/node
    QVector<WaveformWidgetHolder> m_waveformWidgetHolders;

    WaveformWidgetType::Type m_type;

    ConfigObject<ConfigValue>* m_config;

    bool m_skipRender;
    int m_frameRate;
    int m_endOfTrackWarningTime;
    int m_defaultZoom;
    bool m_zoomSync;
    double m_visualGain[FilterCount];
    bool m_overviewNormalized;

    bool m_openGLAvailable;
    QString m_openGLVersion;
    bool m_openGLShaderAvailable;

    VSyncThread* m_vsyncThread;

    //Debug
    QTime m_time;
    float m_frameCnt;
    double m_actualFrameRate;
    int m_vSyncType;
};

#endif // WAVEFORMWIDGETFACTORY_H
