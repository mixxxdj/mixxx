#include "widget/wrecordingduration.h"

WRecordingDuration::WRecordingDuration(QWidget *parent,
                                    RecordingManager* pRecordingManager)
        : WLabel(parent),
          m_pRecordingManager(pRecordingManager) {
}

WRecordingDuration::~WRecordingDuration() {
}

void WRecordingDuration::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);
    connect(m_pRecordingManager, SIGNAL(durationRecorded(QString)),
        this, SLOT(refreshLabel(QString)));
    connect(m_pRecordingManager, SIGNAL(isRecording(bool)),
            this, SLOT(slotRecordingInactive(bool)));
}

void WRecordingDuration::slotRecordingInactive(bool isRecording) {
    // If recording is stopped/inactive
    if(!isRecording) {
        setText("--:--");
    }
}

void WRecordingDuration::refreshLabel(QString durationRecorded) {
    setText(durationRecorded);
}
