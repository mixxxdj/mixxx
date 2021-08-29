#pragma once

#include "audio/frame.h"
#include "control/controlobject.h"

/// Special kind of `ControlObject` that is used for storing frame positions.
//
/// Although it exposes a `mixxx::audio::FramePos`-based API, the underlying
/// control value uses sample positions for backwards compatibility. This might
/// change in the future.
class ControlFramePos : public ControlObject {
    Q_OBJECT
  public:
    /// Creates a new ControlFramePos with the given default position.
    ControlFramePos(const ConfigKey& key, mixxx::audio::FramePos defaultPosition);

    /// Creates a new ControlFramePos. The default position is invalid.
    ControlFramePos(const ConfigKey& key)
            : ControlFramePos(key, mixxx::audio::kInvalidFramePos){};

    /// Set the default position that is used when calling `reset()`.
    void setDefaultPosition(mixxx::audio::FramePos position) {
        ControlObject::setDefaultValue(position.toEngineSamplePosMaybeInvalid());
    }

    /// Returns the default position (i.e. the position that the control object
    /// is set to when calling `reset()`).
    mixxx::audio::FramePos defaultPosition() const {
        return mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                ControlObject::defaultValue());
    }

    /// Returns the `mixxx:audio::FramePos` interpretation of the ControlObject.
    mixxx::audio::FramePos toFramePos() const {
        return mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(ControlObject::get());
    }

    /// Set the ControlObject from a `mixxx::audio::FramePos`.
    void set(mixxx::audio::FramePos position) {
        ControlObject::set(position.toEngineSamplePosMaybeInvalid());
    }

    /// Set the ControlObject from a `mixxx::audio::FramePos`.
    void setAndConfirm(mixxx::audio::FramePos position) {
        ControlObject::setAndConfirm(position.toEngineSamplePosMaybeInvalid());
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
