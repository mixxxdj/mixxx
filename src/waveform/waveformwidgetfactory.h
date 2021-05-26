#pragma once

#include <QObject>
#include <QVector>
#include <vector>

#include "preferences/usersettings.h"
#include "skin/legacy/skincontext.h"
#include "util/performancetimer.h"
#include "util/singleton.h"
#include "waveform/waveform.h"
#include "waveform/widgets/waveformwidgettype.h"

class WVuMeter;
class WWaveformViewer;
class WaveformWidgetAbstract;
class VSyncThread;
class GuiTick;
class VisualsManager;

class WaveformWidgetAbstractHandle {
  public:
    WaveformWidgetAbstractHandle();

    WaveformWidgetType::Type getType() const { return m_type;}
    QString getDisplayName() const { return m_displayString;}

  private:
    WaveformWidgetType::Type m_type;
    QString m_displayString;

    friend class WaveformWidgetFactory;
};

class WaveformWidgetHolder {
  public:
    WaveformWidgetHolder();
    WaveformWidgetHolder(WaveformWidgetHolder&&) = default;
    WaveformWidgetHolder& operator=(WaveformWidgetHolder&&) = default;
  private:
    WaveformWidgetHolder(
            WaveformWidgetAbstract* waveformWidget,
            WWaveformViewer* waveformViewer,
            const QDomNode& skinNode,
            const SkinContext& parentContext);

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

    /// Creates the waveform widget using the type set with setWidgetType
    /// and binds it to the viewer.
    /// Deletes older widget and resets positions to config defaults.
    bool setWaveformWidget(
            WWaveformViewer* viewer,
            const QDomElement &node,
            const SkinContext& parentContext);

    void setFrameRate(int frameRate);
    int getFrameRate() const { return m_frameRate;}
//    bool getVSync() const { return m_vSyncType;}
    void setEndOfTrackWarningTime(int endTime);
    int getEndOfTrackWarningTime() const { return m_endOfTrackWarningTime;}

    bool isOpenGlAvailable() const { return m_openGlAvailable;}
    bool isOpenGlesAvailable() const { return m_openGlesAvailable;}
    QString getOpenGLVersion() const { return m_openGLVersion;}

    bool isOpenGlShaderAvailable() const { return m_openGLShaderAvailable;}

    /// Sets the widget type and saves it to configuration.
    /// Returns false and sets EmtpyWaveform if type is invalid
    bool setWidgetType(WaveformWidgetType::Type type);
    /// Changes the widget type to that loaded from config and recreates them.
    /// Used as a workaround on Windows due to a problem with GL and QT 5.14.2
    bool setWidgetTypeFromConfig();
    /// Changes the widget type and recreates them. Used from the preferences
    /// dialog.
    bool setWidgetTypeFromHandle(int handleIndex, bool force = false);
    WaveformWidgetType::Type getType() const { return m_type;}

  protected:
    bool setWidgetType(
            WaveformWidgetType::Type type,
            WaveformWidgetType::Type* pCurrentType);

  public:
    void setDefaultZoom(double zoom);
    double getDefaultZoom() const { return m_defaultZoom;}

    void setZoomSync(bool sync);
    int isZoomSync() const { return m_zoomSync;}

    void setDisplayBeatGridAlpha(int alpha);
    int getBeatGridAlpha() const { return m_beatGridAlpha; }

    void setVisualGain(FilterIndex index, double gain);
    double getVisualGain(FilterIndex index) const;

    void setOverviewNormalized(bool normalize);
    int isOverviewNormalized() const { return m_overviewNormalized;}

    const QVector<WaveformWidgetAbstractHandle> getAvailableTypes() const { return m_waveformWidgetHandles;}
    void getAvailableVSyncTypes(QList<QPair<int, QString > >* list);
    void destroyWidgets();

    void addTimerListener(WVuMeter* pWidget);

    void startVSync(GuiTick* pGuiTick, VisualsManager* pVisualsManager);
    void setVSyncType(int vsType);
    int getVSyncType();

    void setPlayMarkerPosition(double position);
    double getPlayMarkerPosition() const { return m_playMarkerPosition; }

    void notifyZoomChange(WWaveformViewer *viewer);

    WaveformWidgetType::Type autoChooseWidgetType() const;

  signals:
    void waveformUpdateTick();
    void waveformMeasured(float frameRate, int droppedFrames);
    void renderSpinnies(VSyncThread*);
    void swapSpinnies();

  public slots:
    void slotSkinLoaded();

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

    WaveformWidgetType::Type findTypeFromHandleIndex(int index);
    int findHandleIndexFromType(WaveformWidgetType::Type type);

    //All type of available widgets

    QVector<WaveformWidgetAbstractHandle> m_waveformWidgetHandles;

    //Currently in use widgets/visual/node
    std::vector<WaveformWidgetHolder> m_waveformWidgetHolders;

    WaveformWidgetType::Type m_type;
    WaveformWidgetType::Type m_configType;

    UserSettingsPointer m_config;

    bool m_skipRender;
    int m_frameRate;
    int m_endOfTrackWarningTime;
    double m_defaultZoom;
    bool m_zoomSync;
    double m_visualGain[FilterCount];
    bool m_overviewNormalized;

    bool m_openGlAvailable;
    bool m_openGlesAvailable;
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
