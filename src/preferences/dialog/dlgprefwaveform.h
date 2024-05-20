#pragma once

#include <memory>

#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefwaveformdlg.h"
#include "preferences/usersettings.h"
#include "waveform/widgets/waveformwidgettype.h"
#ifdef MIXXX_USE_QOPENGL
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"
#define DECL_SLOT_WAVEFORM_OPTION(opt)                                               \
    void slotSetWaveformOption##opt(bool checked) {                                  \
        slotSetWaveformOptions(allshader::WaveformRendererSignalBase::opt, checked); \
    }
#endif

class Library;

class DlgPrefWaveform : public DlgPreferencePage, public Ui::DlgPrefWaveformDlg {
    Q_OBJECT
  public:
    DlgPrefWaveform(
            QWidget* pParent,
            UserSettingsPointer pConfig,
            std::shared_ptr<Library> pLibrary);
    virtual ~DlgPrefWaveform();

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
    void slotSetWaveformOptions(allshader::WaveformRendererSignalBase::Option option, bool enabled);
    DECL_SLOT_WAVEFORM_OPTION(SplitStereoSignal);
    DECL_SLOT_WAVEFORM_OPTION(HighDetails);
#endif
    void slotSetWaveformOverviewType(int index);
    void slotSetDefaultZoom(int index);
    void slotSetZoomSynchronization(bool checked);
    void slotSetVisualGainAll(double gain);
    void slotSetVisualGainLow(double gain);
    void slotSetVisualGainMid(double gain);
    void slotSetVisualGainHigh(double gain);
    void slotSetNormalizeOverview(bool normalize);
    void slotWaveformMeasured(float frameRate, int droppedFrames);
    void slotClearCachedWaveforms();
    void slotSetBeatGridAlpha(int alpha);
    void slotSetPlayMarkerPosition(int position);
    void slotSetUntilMarkShowBeats(bool checked);
    void slotSetUntilMarkShowTime(bool checked);
    void slotSetUntilMarkAlign(int index);
    void slotSetUntilMarkTextPointSize(int value);
  signals:
    void reloadUserInterface();

  private:
    void initWaveformControl();
    void calculateCachedWaveformDiskUsage();
    void notifyRebootNecessary();
    void updateEnableUntilMark();
    void updateWaveformOption(
            bool useWaveform, WaveformWidgetBackend backend, int currentOptions);
    void updateWaveformAcceleration(
            WaveformWidgetType::Type type, WaveformWidgetBackend backend);

    UserSettingsPointer m_pConfig;
    std::shared_ptr<Library> m_pLibrary;
};

#ifdef DECL_SLOT_WAVEFORM_OPTION
#undef DECL_SLOT_WAVEFORM_OPTION
#endif
