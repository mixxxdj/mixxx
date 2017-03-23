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
    connect(m_pRecordingManager, SIGNAL(isRecordingActive(bool)),
        this, SLOT(clearLabel(bool)));
}

void WRecordingDuration::clearLabel(bool toggle) {
    Q_UNUSED(toggle);
    // If recording is stopped/inactive
    if(!m_pRecordingManager->isRecordingActive()) {
        setText("--:--");
    }
}

void WRecordingDuration::refreshLabel(QString durationRecorded) {
    setText(durationRecorded);
}
