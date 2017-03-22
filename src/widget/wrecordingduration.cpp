#include "widget/wrecordingduration.h"

WRecordingDuration::WRecordingDuration(QWidget *parent,
                                    RecordingManager* pRecordingManager)
        : WLabel(parent),
          m_durationRecordedStr(""),
          m_pRecordingManager(pRecordingManager) {
}

WRecordingDuration::~WRecordingDuration() {
}

void WRecordingDuration::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);
    connect(m_pRecordingManager, SIGNAL(durationRecorded(QString)),
        this, SLOT(refreshLabelText(QString)));
}

void WRecordingDuration::refreshLabelText(QString durationRecorded) {
    m_durationRecordedStr = durationRecorded;
    setText(m_durationRecordedStr);
}
