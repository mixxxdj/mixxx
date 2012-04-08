#ifndef WAVEFORMSIGNALCOLORS_H
#define WAVEFORMSIGNALCOLORS_H

#include <QColor>

class QDomNode;

class WaveformSignalColors
{
public:
    WaveformSignalColors();
    virtual ~WaveformSignalColors() {}

    bool setup(const QDomNode &node);

    const QColor& getSignalColor() const { return m_signalColor;}
    const QColor& getLowColor() const { return m_lowColor;}
    const QColor& getMidColor() const { return m_midColor;}
    const QColor& getHighColor() const { return m_highColor;}

protected:
    void fallBackFromSignalColor();
    void fallBackDefaultColor();

    float stableHue( float hue) const;

private:
    QColor m_signalColor;
    QColor m_lowColor;
    QColor m_midColor;
    QColor m_highColor;
};

#endif // WAVEFORMSIGNALCOLORS_H
