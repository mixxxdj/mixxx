#ifndef DLGPREFWAVEFORM_H
#define DLGPREFWAVEFORM_H

#include <QWidget>

#include "preferences/dialog/ui_dlgprefwaveformdlg.h"
#include "preferences/usersettings.h"
#include "preferences/dlgpreferencepage.h"

class MixxxMainWindow;
class Library;

class DlgPrefWaveform : public DlgPreferencePage, public Ui::DlgPrefWaveformDlg {
    Q_OBJECT
  public:
    DlgPrefWaveform(QWidget* pParent, MixxxMainWindow* pMixxx,
                    UserSettingsPointer pConfig, Library* pLibrary);
    virtual ~DlgPrefWaveform();

  public slots:
    void slotUpdate() override;
    void slotApply() override;
    void slotResetToDefaults() override;
    void slotSetWaveformEndRender(int endTime);

  private slots:
    void slotSetFrameRate(int frameRate);
    void slotSetWaveformType(int index);
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

  private:
    void initWaveformControl();
    void calculateCachedWaveformDiskUsage();

    UserSettingsPointer m_pConfig;
    Library* m_pLibrary;
    MixxxMainWindow* m_pMixxx;
};


#endif /* DLGPREFWAVEFORM_H */
