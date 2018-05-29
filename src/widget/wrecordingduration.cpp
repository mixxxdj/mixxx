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

    // When we're recording show text from "InactiveText" node
    QString inactiveText;
    if (context.hasNodeSelectString(node, "InactiveText", &inactiveText)) {
        m_inactiveText = inactiveText;
    } else {
        m_inactiveText = QString("--:--");
    }
    // Set inactiveText here already because slotRecordingInactive
    // is refreshed first when we start recording
    setText(m_inactiveText);
}

void WRecordingDuration::slotRecordingInactive(bool isRecording) {
    // When we're not recording show InactiveText
    if(!isRecording) {
        setText(m_inactiveText);
    }
}

void WRecordingDuration::refreshLabel(QString durationRecorded) {
    setText(durationRecorded);
}
