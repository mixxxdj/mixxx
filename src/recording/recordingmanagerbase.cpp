#include "recordingmanagerbase.h"

void RecordingManagerBase::slotSetRecording(bool record) {
    if (record && !isRecordingActive()) {
        startRecording();
    } else if (!record && isRecordingActive()) {
        stopRecording();
    }
}

void RecordingManagerBase::slotToggleRecording(double value) {
    if (value == 0) {
        return;
    }
    if (isRecordingActive()) {
        stopRecording();
    } else {
        startRecording();
    }
}
