#include "recordingmanagerbase.h"

void RecordingManagerBase::slotSetRecording(bool recording) {
    if (recording && !isRecordingActive()) {
        startRecording();
    } else if (!recording && isRecordingActive()) {
        stopRecording();
    }
}

void RecordingManagerBase::slotToggleRecording(double value) {
    if (!value) {
        return;
    }
    if (isRecordingActive()) {
        stopRecording();
    } else {
        startRecording();
    }
}
