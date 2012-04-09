#ifndef WAVEFORMMARKRANGE_H
#define WAVEFORMMARKRANGE_H

#include <QPixmap>

class ControlObject;

class WaveformMarkRange
{
public:
    WaveformMarkRange();

    bool isValid() const { return m_markStartPoint && m_markEndPoint;}
    bool isActive() const;

private:
    void generatePixmap(int weidth, int height);

    ControlObject* m_markStartPoint;
    ControlObject* m_markEndPoint;
    ControlObject* m_markEnabled;

    QColor m_activeColor;
    QColor m_disabledColor;

    QPixmap m_activePixmap;
    QPixmap m_disabledPixmap;

    friend class WaveformRenderMarkRange;
};

#endif // WAVEFORMMARKRANGE_H
