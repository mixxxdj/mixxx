#include "control/controlframepos.h"

#include "moc_controlframepos.cpp"

ControlFramePos::ControlFramePos(const ConfigKey& key, mixxx::audio::FramePos defaultPosition)
        : ControlObject(key) {
    setDefaultPosition(defaultPosition);
    reset();
    connect(this,
            &ControlFramePos::valueChanged,
            this,
            &ControlFramePos::slotValueChanged,
            Qt::DirectConnection);
}

void ControlFramePos::slotValueChanged(double value) {
    const auto position = mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(value);
    emit positionChanged(position);
}
