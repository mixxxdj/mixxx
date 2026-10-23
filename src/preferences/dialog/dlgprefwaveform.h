#pragma once

#include <memory>

#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefwaveformdlg.h"
#include "preferences/usersettings.h"
#include "waveform/widgets/waveformwidgettype.h"
#ifdef MIXXX_USE_QOPENGL
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"
#endif

class ControlPushButton;
class ControlObject;
class Library;

class DlgPrefWaveform : public DlgPreferencePage, public Ui::DlgPrefWaveformDlg {
    Q_OBJECT
  public:
    DlgPrefWaveform(
            QWidget* pParent,
            UserSettingsPointer pConfig,
            std::shared_ptr<Library> pLibrary);

  public slots:
    void slotUpdate() override;
    void slotApply() override;
    void slotResetToDefaults() override;
    void slotSetWaveformEndRender(int endTime);

  private slots:
    void slotSetFrameRate(int frameRate);
    void slotSetWaveformType(int index);
    void slotSetWaveformEnabled(bool checked);
    void slotSetWaveformAcceleration(bool checked);
#ifdef MIXXX_USE_QOPENGL
    void slotSetWaveformOptions(WaveformRendererSignalBase::Option option, bool enabled);
    void slotSetWaveformOptionSplitStereoSignal(bool checked) {
        slotSetWaveformOptions(WaveformRendererSignalBase::Option::
                                       SplitStereoSignal,
                checked);
    }
    void slotSetWaveformOptionHighDetail(bool checked) {
        slotSetWaveformOptions(WaveformRendererSignalBase::Option::HighDetail, checked);
    }
#endif
    void slotSetDefaultZoom(int index);
    void slotSetZoomSynchronization(bool checked);
    void slotSetVisualGainAll(double gain);
    void slotSetVisualGainLow(double gain);
    void slotSetVisualGainMid(double gain);
    void slotSetVisualGainHigh(double gain);
    void slotWaveformMeasured(float frameRate, int droppedFrames);
    void slotClearCachedWaveforms();
    void slotSetBeatGridAlpha(int alpha);
    void slotSetPlayMarkerPosition(int position);
    void slotSetUntilMarkShowBeats(bool checked);
    void slotSetUntilMarkShowTime(bool checked);
    void slotSetUntilMarkAlign(int index);
    void slotSetUntilMarkTextPointSize(int value);
    void slotSetUntilMarkTextHeightLimit(int index);
    void slotStemOpacity(double value);
    void slotStemReorderOnChange(bool value);
    void slotStemOutlineOpacity(double value);
    void slotStemDisplayMode(int index);
    // overview options
    void slotSetWaveformOverviewType();
    void slotSetOverviewStereoMode(bool mono);
    void slotSetOverviewMinuteMarkers(bool minuteMarkers);
    void slotSetOverviewScaling();

    // BPM & Key display options
    void slotSetShowBpmCurve(bool checked);
    void slotSetShowBpmMarkers(bool checked);
    void slotSetShowBpmLabels(bool checked);
    void slotSetShowKeyMarkers(bool checked);
    void slotSetShowKeyLabels(bool checked);
    void slotSetShowLancelotWheel(bool checked);
    void slotSetOverviewShowBpmCurve(bool checked);
    void slotSetOverviewShowBpmMarkers(bool checked);
    void slotSetOverviewShowKeyMarkers(bool checked);

  private:
    void initWaveformControl();
    void calculateCachedWaveformDiskUsage();
    void notifyRebootNecessary();
    void updateEnableUntilMark();
    void updateWaveformTypeOptions(bool useWaveform);
    void updateWaveformAcceleration(WaveformWidgetType::Type type);
    void updateWaveformGeneralOptionsEnabled();
    void updateWaveformGainEnabled();
    void updateStemOptionsEnabled();
    void notifyQmlWaveformSettingsChanged();

    std::unique_ptr<ControlPushButton> m_pTypeControl;
    std::unique_ptr<ControlObject> m_pOverviewMinuteMarkersControl;
    std::unique_ptr<ControlObject> m_pOverviewStereoControl;

    // BPM & Key display ControlProxies
    std::unique_ptr<ControlObject> m_pShowBpmCurve;
    std::unique_ptr<ControlObject> m_pShowBpmMarkers;
    std::unique_ptr<ControlObject> m_pShowBpmLabels;
    std::unique_ptr<ControlObject> m_pShowKeyMarkers;
    std::unique_ptr<ControlObject> m_pShowKeyLabels;
    std::unique_ptr<ControlObject> m_pShowLancelotWheel;
    std::unique_ptr<ControlObject> m_pOverviewShowBpmCurve;
    std::unique_ptr<ControlObject> m_pOverviewShowBpmMarkers;
    std::unique_ptr<ControlObject> m_pOverviewShowKeyMarkers;

    UserSettingsPointer m_pConfig;
    std::shared_ptr<Library> m_pLibrary;
};
