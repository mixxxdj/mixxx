#ifndef DLGPREFRECORD_H
#define DLGPREFRECORD_H

#include <QRadioButton>
#include <QWidget>

#include "preferences/dialog/ui_dlgprefrecorddlg.h"
#include "preferences/usersettings.h"
#include "preferences/dlgpreferencepage.h"
#include "encoder/encoder.h"
#include "util/widgethider.h"

class ControlObject;
class ControlProxy;

class DlgPrefRecord : public DlgPreferencePage, public Ui::DlgPrefRecordDlg  {
    Q_OBJECT
    WidgetHider m_hider;
  public:
    DlgPrefRecord(QWidget *parent, UserSettingsPointer _config);
    virtual ~DlgPrefRecord();

  public slots:
    // Apply changes to widget
    void slotApply();
    void slotUpdate();
    void slotResetToDefaults();

    // Dialog to browse for recordings directory
    void slotBrowseRecordingsDir();

    void slotFormatChanged();
    void slotSliderQuality();
    void slotSliderCompression();
    void slotGroupChanged();

  signals:
    void apply(const QString &);

  private:
    void retainSizeFor(QWidget* widget);
    inline void showWidget(QWidget* widget);
    inline void hideWidget(QWidget* widget);
    void setupEncoderUI(Encoder::Format selformat);
    void loadMetaData();
    void updateTextQuality();
    void updateTextCompression();
    void saveRecordingFolder();
    void saveMetaData();
    void saveEncoding();
    void saveUseCueFile();
    void saveSplitSize();

    // Pointer to config object
    UserSettingsPointer m_pConfig;
    Encoder::Format m_selFormat;
    QButtonGroup encodersgroup;
    QButtonGroup optionsgroup;
    QList<QRadioButton*> m_formatButtons;
    QList<QAbstractButton*> m_optionWidgets;
};

#endif
