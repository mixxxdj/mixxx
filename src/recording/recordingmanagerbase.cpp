#include "recordingmanagerbase.h"

void RecordingManagerBase::slotSetRecording(bool recording) {
    if (recording && !isRecordingActive()) {
        startRecording();
    } else if (!recording && isRecordingActive()) {
        stopRecording();
    }
}

void RecordingManagerBase::slotToggleRecording(double value) {
    bool toggle = static_cast<bool>(value);
    if (toggle) {
        if (isRecordingActive()) {
            stopRecording();
        } else {
            startRecording();
        }
    }
}

bool RecordingManagerBase::isRecordingActive() const {
    return m_bRecording;
}
