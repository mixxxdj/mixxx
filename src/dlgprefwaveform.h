#ifndef DLGPREFWAVEFORM_H
#define DLGPREFWAVEFORM_H

#include <QWidget>

#include "ui_dlgprefwaveformdlg.h"
#include "configobject.h"
#include "preferences/dlgpreferencepage.h"

class MixxxMainWindow;

class DlgPrefWaveform : public DlgPreferencePage, public Ui::DlgPrefWaveformDlg {
    Q_OBJECT
  public:
    DlgPrefWaveform(QWidget* pParent, MixxxMainWindow* pMixxx,
                    ConfigObject<ConfigValue> *pConfig);
    virtual ~DlgPrefWaveform();

  public slots:
    void slotUpdate();
    void slotApply();
    void slotResetToDefaults();
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

  private:
    void initWaveformControl();

    ConfigObject<ConfigValue>* m_pConfig;
    MixxxMainWindow* m_pMixxx;
};


#endif /* DLGPREFWAVEFORM_H */
