#include "widget/wrecordingduration.h"

WRecordignDuration::WRecordignDuration(QWidget *parent)
        : WLabel(parent),
    connect(m_pRecordingManager, SIGNAL(durationRecorded(QString)),
            this, SLOT(slotDurationRecorded(QString)));
}

WRecordignDuration::~WRecordignDuration() {
}

void WRecordignDuration::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);
}

void WRecordignDuration::slotDurationRecorded(QString durationRecorded) {
    m_durationRecordedStr = durationRecorded;
    refreshLabel();
}

void WRecordignDuration::refreshLabel() {
    QString recDuration = m_durationRecordedStr;
    label->setText(recDuration);
}
