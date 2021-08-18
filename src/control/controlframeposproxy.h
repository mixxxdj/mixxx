#pragma once

#include "audio/frame.h"
#include "control/controlproxy.h"

/// Special kind of `ControlProxy` that is used for storing frame positions.
//
/// Although it exposes a `mixxx::audio::FramePos`-based API, the underlying
/// control value uses sample positions for backwards compatibility. This might
/// change in the future.
class ControlFramePosProxy : public ControlProxy {
    Q_OBJECT
  public:
    /// Creates a new ControlFramePos with the given default position.
    ControlFramePosProxy(const ConfigKey& key, QObject* pParent = nullptr);

    /// Returns the `mixxx:audio::FramePos` interpretation of the ControlProxy.
    mixxx::audio::FramePos toFramePos() const {
        return mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(ControlProxy::get());
    }

    /// Set the ControlProxy from a `mixxx::audio::FramePos`.
    void set(mixxx::audio::FramePos position) {
        ControlProxy::set(position.toEngineSamplePosMaybeInvalid());
    }

  signals:
    /// Emitted whenever the `valueChanged` signal is emitted. Connect to this
    /// signal when you want to receive the `mixxx::audio::FramePos` instead of
    /// the double sample position.
    void positionChanged(mixxx::audio::FramePos position);

  private slots:
    /// Invoked by the `valueChanged` signal. Converts the new value to a
    /// `mixxx::audio::FramePos` and emit the `positionChanged` signal.
    void slotValueChanged(double value);
};
