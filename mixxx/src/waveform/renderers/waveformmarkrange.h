#ifndef WAVEFORMMARKRANGE_H
#define WAVEFORMMARKRANGE_H

#include <QPixmap>

class ControlObjectThreadMain;
class QDomNode;

class WaveformMarkRange {
  public:
    WaveformMarkRange();

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

    void setup(const QString &group, const QDomNode& node);

  private:
    void generatePixmap(int weidth, int height);

    ControlObjectThreadMain* m_markStartPointControl;
    ControlObjectThreadMain* m_markEndPointControl;
    ControlObjectThreadMain* m_markEnabledControl;

    QColor m_activeColor;
    QColor m_disabledColor;

    QPixmap m_activePixmap;
    QPixmap m_disabledPixmap;

    friend class WaveformRenderMarkRange;
    friend class WOverview;
};

#endif // WAVEFORMMARKRANGE_H
