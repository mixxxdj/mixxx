#pragma once

#include <QPoint>

/// This interface specifies functions to be implemented by waveform component renderers
/// to make them clickable.
class WaveformElementRightClickable {
  public:
    /// Expects an implementation of boundary check for the waveform graphic element.
    virtual bool contains(QPoint point, Qt::Orientation orientation) const = 0;
    virtual ~WaveformElementRightClickable() = default;
};
