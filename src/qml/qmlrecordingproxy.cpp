#include "qml/qmlrecordingproxy.h"

#include <utility>

#include "moc_qmlrecordingproxy.cpp"
#include "recording/recordingmanager.h"

namespace mixxx {
namespace qml {

namespace {
const QString kInactiveDurationText = QStringLiteral("--:--");
} // namespace

QmlRecordingProxy::QmlRecordingProxy(
        std::shared_ptr<RecordingManager> pRecordingManager,
        QObject* parent)
        : QObject(parent),
          m_pRecordingManager(std::move(pRecordingManager)),
          m_durationText(kInactiveDurationText) {
    if (!m_pRecordingManager) {
        return;
    }

    connect(m_pRecordingManager.get(),
            &RecordingManager::durationRecorded,
            this,
            &QmlRecordingProxy::slotDurationRecorded);
    connect(m_pRecordingManager.get(),
            &RecordingManager::isRecording,
            this,
            &QmlRecordingProxy::slotRecordingActiveChanged);

    if (!m_pRecordingManager->isRecordingActive()) {
        slotRecordingActiveChanged(false);
    }
}

QmlRecordingProxy* QmlRecordingProxy::create(
        QQmlEngine* pQmlEngine,
        [[maybe_unused]] QJSEngine* pJsEngine) {
    return new QmlRecordingProxy(s_pRecordingManager, pQmlEngine);
}

void QmlRecordingProxy::slotDurationRecorded(const QString& durationText) {
    if (m_durationText == durationText) {
        return;
    }
    m_durationText = durationText;
    emit durationTextChanged();
}

void QmlRecordingProxy::slotRecordingActiveChanged(bool isRecording) {
    if (isRecording || m_durationText == kInactiveDurationText) {
        return;
    }
    m_durationText = kInactiveDurationText;
    emit durationTextChanged();
}

} // namespace qml
} // namespace mixxx
