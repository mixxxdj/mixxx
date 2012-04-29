#ifndef WAVEFORMMARKRANGE_H
#define WAVEFORMMARKRANGE_H

#include <QPixmap>

class ControlObject;
class QDomNode;

class WaveformMarkRange
{
public:
    WaveformMarkRange();

    bool isValid() const { return m_markStartPointControl && m_markEndPointControl;}
    bool isActive() const;

    void setup(const QString &group, const QDomNode& node);

private:
    void generatePixmap(int weidth, int height);

    ControlObject* m_markStartPointControl;
    ControlObject* m_markEndPointControl;
    ControlObject* m_markEnabledControl;

    QColor m_activeColor;
    QColor m_disabledColor;

    QPixmap m_activePixmap;
    QPixmap m_disabledPixmap;

    friend class WaveformRenderMarkRange;
    friend class WOverview;
};

#endif // WAVEFORMMARKRANGE_H
