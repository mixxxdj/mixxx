#pragma once

#include <QColor>
#include <QImage>
#include <QString>

#include "control/controlproxy.h"
#include "util/memory.h"

class QDomNode;
class SkinContext;
class WaveformSignalColors;

class WaveformMarkRange {
  public:
    WaveformMarkRange(
            const QString& group,
            const QDomNode& node,
            const SkinContext& context,
            const WaveformSignalColors& signalColors);
    // This class is only moveable, but not copyable!
    WaveformMarkRange(WaveformMarkRange&&) = default;
    WaveformMarkRange(const WaveformMarkRange&) = delete;

    // If a mark range is active it has valid start/end points so it should be
    // drawn on waveforms.
    bool active() const;
    // If a mark range is enabled that means it should be painted with its
    // active color instead of its disabled color.
    bool enabled() const;
    // Returns start value or -1 if the start control doesn't exist.
    double start() const;
    // Returns end value or -1 if the end control doesn't exist.
    double end() const;

  private:
    void generateImage(int weidth, int height);

    std::unique_ptr<ControlProxy> m_markStartPointControl;
    std::unique_ptr<ControlProxy> m_markEndPointControl;
    std::unique_ptr<ControlProxy> m_markEnabledControl;

    QColor m_activeColor;
    QColor m_disabledColor;

    QImage m_activeImage;
    QImage m_disabledImage;

    friend class WaveformRenderMarkRange;
    friend class WOverview;
};
