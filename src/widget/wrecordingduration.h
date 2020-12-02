// WRecordingDuration is a widget showing the duration of running recoding
// In skins it is represented by a <RecordingDuration> node.

#pragma once

#include "widget/wlabel.h"
#include "skin/skincontext.h"
#include "recording/recordingmanager.h"

class WRecordingDuration: public WLabel {
    Q_OBJECT
  public:
    WRecordingDuration(QWidget* parent, RecordingManager* pRecordingManager);
    ~WRecordingDuration() override;

    void setup(const QDomNode& node, const SkinContext& context) override;

  private slots:
    void refreshLabel(const QString&);
    void slotRecordingInactive(bool);

  private:
    RecordingManager* m_pRecordingManager;
    QString m_inactiveText;
};
