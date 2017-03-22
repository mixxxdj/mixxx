#include "widget/wrecordingduration.h"

WRecordingDuration::WRecordingDuration(QWidget *parent,
                                    RecordingManager* pRecordingManager)
        : WLabel(parent),
          m_durationRecordedStr(""),
          m_pRecordingManager(pRecordingManager) {
        connect(m_pRecordingManager, SIGNAL(durationRecorded(QString)),
            this, SLOT(slotDurationRecorded(QString)));
}

WRecordingDuration::~WRecordingDuration() {
}

void WRecordingDuration::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);
}

void WRecordingDuration::slotDurationRecorded(QString durationRecorded) {
    m_durationRecordedStr = durationRecorded;
    refreshLabel();
}

void WRecordingDuration::refreshLabel() {
    QString recDuration = m_durationRecordedStr;
    label->setText(recDuration);
}
