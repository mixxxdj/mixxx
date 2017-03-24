// wrecordingduration.h
// WRecordingDuration is a label showing the duration of running recording.
// In skin.xml, it is represented by a <RecordingDuration> node.

#ifndef WRECORDINGDURATION_H
#define WRECORDINGDURATION_H

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
    void refreshLabelText(QString);

  private:
    QString m_durationRecordedStr;

    RecordingManager* m_pRecordingManager;
};

#endif /* WRECORDINGDURATION_H */
