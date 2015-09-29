#ifndef WAVEFORMMARKRANGE_H
#define WAVEFORMMARKRANGE_H

#include <QImage>

#include "skin/skincontext.h"

class ControlObjectSlave;
class QDomNode;
class WaveformSignalColors;

class WaveformMarkRange {
  public:
    WaveformMarkRange();
    ~WaveformMarkRange();

    // If a mark range is active it has valid start/end points so it should be
    // drawn on waveforms.
    bool active();
    // If a mark range is enabled that means it should be painted with its
    // active color instead of its disabled color.
    bool enabled();
    // Returns start value or -1 if the start control doesn't exist.
    double start();
    // Returns end value or -1 if the end control doesn't exist.
    double end();

    void setup(const QString &group, const QDomNode& node,
               const SkinContext& context,
               const WaveformSignalColors& signalColors);

  private:
    void generateImage(int weidth, int height);

    ControlObjectSlave* m_markStartPointControl;
    ControlObjectSlave* m_markEndPointControl;
    ControlObjectSlave* m_markEnabledControl;

    QColor m_activeColor;
    QColor m_disabledColor;

    QImage m_activeImage;
    QImage m_disabledImage;

    friend class WaveformRenderMarkRange;
    friend class WOverview;
};

#endif // WAVEFORMMARKRANGE_H
