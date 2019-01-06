#ifndef WAVEFORMWIDGETFACTORY_H
#define WAVEFORMWIDGETFACTORY_H

#include <QObject>
#include <QTime>
#include <QVector>

#include "util/singleton.h"
#include "preferences/usersettings.h"
#include "waveform/widgets/waveformwidgettype.h"
#include "waveform/waveform.h"
#include "skin/skincontext.h"
#include "util/performancetimer.h"

class WWaveformViewer;
class WaveformWidgetAbstract;
class QTimer;
class VSyncThread;
class GuiTick;
class VisualsManager;

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
    //TODO merge this enum with the waveform analyzer one
    enum FilterIndex { All = 0, Low = 1, Mid = 2, High = 3, FilterCount = 4};

    bool setConfig(UserSettingsPointer config);

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

    void setDefaultZoom(double zoom);
    double getDefaultZoom() const { return m_defaultZoom;}

    void setZoomSync(bool sync);
    int isZoomSync() const { return m_zoomSync;}

    void setDisplayBeatGridAlpha(int alpha);
    int beatGridAlpha() const { return m_beatGridAlpha; }

    void setVisualGain(FilterIndex index, double gain);
    double getVisualGain(FilterIndex index) const;

    void setOverviewNormalized(bool normalize);
    int isOverviewNormalized() const { return m_overviewNormalized;}

    const QVector<WaveformWidgetAbstractHandle> getAvailableTypes() const { return m_waveformWidgetHandles;}
    void getAvailableVSyncTypes(QList<QPair<int, QString > >* list);
    void destroyWidgets();

    void addTimerListener(QWidget* pWidget);

    void startVSync(GuiTick* pGuiTick, VisualsManager* pVisualsManager);
    void setVSyncType(int vsType);
    int getVSyncType();

    void setPlayMarkerPosition(double position);
    double getPlayMarkerPosition() const { return m_playMarkerPosition; }

    void notifyZoomChange(WWaveformViewer *viewer);

    WaveformWidgetType::Type autoChooseWidgetType() const;

    // Returns the devicePixelRatio for the current window. This is the scaling
    // factor between screen pixels and "device independent pixels". For
    // example, on macOS with a retina display the ratio is 2.
    static float getDevicePixelRatio();

  signals:
    void waveformUpdateTick();
    void waveformMeasured(float frameRate, int droppedFrames);
    void renderSpinnies();
    void swapSpinnies();

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

    UserSettingsPointer m_config;

    bool m_skipRender;
    int m_frameRate;
    int m_endOfTrackWarningTime;
    double m_defaultZoom;
    bool m_zoomSync;
    double m_visualGain[FilterCount];
    bool m_overviewNormalized;

    bool m_openGLAvailable;
    QString m_openGLVersion;
    bool m_openGLShaderAvailable;
    int m_beatGridAlpha;

    VSyncThread* m_vsyncThread;
    GuiTick* m_pGuiTick;  // not owned
    VisualsManager* m_pVisualsManager;  // not owned

    //Debug
    PerformanceTimer m_time;
    float m_frameCnt;
    double m_actualFrameRate;
    int m_vSyncType;
    double m_playMarkerPosition;
};

#endif // WAVEFORMWIDGETFACTORY_H
