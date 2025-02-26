#pragma once

#include <QColor>
#include <QImage>
#include <QString>
#include <memory>

#include "control/controlproxy.h"
#include "waveform/waveformmarklabel.h"

QT_FORWARD_DECLARE_CLASS(QDomNode);

class SkinContext;
class WaveformSignalColors;

namespace allshader {
class WaveformRenderMarkRange;
}

class WaveformMarkRange {
  public:
    WaveformMarkRange(
            const QString& group,
            const QDomNode& node,
            const SkinContext& context,
            const WaveformSignalColors& signalColors);
    // This class is only moveable, but not copiable!
    WaveformMarkRange(WaveformMarkRange&&) = default;
    WaveformMarkRange(const WaveformMarkRange&) = delete;

    // If a mark range is active it has valid start/end points so it should be
    // drawn on waveforms.
    bool active() const;
    // If a mark range is enabled that means it should be painted with its
    // active color instead of its disabled color.
    bool enabled() const;
    // If a mark range is visible it should be drawn, otherwise it should be
    // hidden, regardless whether it is active or not.
    bool visible() const;
    // Returns start value or -1 if the start control doesn't exist.
    double start() const;
    // Returns end value or -1 if the end control doesn't exist.
    double end() const;

    bool showDuration() const;

    enum class DurationTextLocation {
        Before = 0,
        After = 1
    };

    DurationTextLocation durationTextLocation() const {
        return m_durationTextLocation;
    }

    WaveformMarkLabel m_durationLabel;

  private:
    void generateImage(int width, int height);

    std::unique_ptr<ControlProxy> m_markStartPointControl;
    std::unique_ptr<ControlProxy> m_markEndPointControl;
    std::unique_ptr<ControlProxy> m_markEnabledControl;
    std::unique_ptr<ControlProxy> m_markVisibleControl;

    QColor m_activeColor;
    QColor m_disabledColor;
    double m_enabledOpacity;
    double m_disabledOpacity;
    QColor m_durationTextColor;

    QImage m_activeImage;
    QImage m_disabledImage;

    DurationTextLocation m_durationTextLocation;

    friend class WaveformRenderMarkRange;
    friend class allshader::WaveformRenderMarkRange;
    friend class WOverview;
};
