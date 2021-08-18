#include "control/controlframepos.h"
#include "moc_controlframeposproxy.cpp"

ControlFramePosProxy::ControlFramePosProxy(const ConfigKey& key, QObject* pParent)
        : ControlProxy(key, pParent) {
    connect(this,
            &ControlFramePosProxy::valueChanged,
            this,
            &ControlFramePosProxy::slotValueChanged,
            Qt::DirectConnection);
}

void ControlFramePosProxy::slotValueChanged(double value) {
    const auto position = mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(value);
    emit positionChanged(position);
}
