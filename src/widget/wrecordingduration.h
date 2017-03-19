// wrecordingduration.h
// WRecordingDuration is a widget showing the duration of running recoding
// In skin.xml, it is represented by a <RecordingDuration> node.

#ifndef WRECORDINGDURATION_H
#define WRECORDINGDURATION_H

#include "widget/wlabel.h"
#include "skin/skincontext.h"

class WRecordingDuration: public WLabel {
    Q_OBJECT
  public:
    explicit WRecordingDuration(QWidget *parent=nullptr);
    ~WRecordingDuration() override;

    void setup(const QDomNode& node, const SkinContext& context) override;

  private slots:
    void refreshLabel();
};

#endif /* WRECORDINGDURATION_H */
