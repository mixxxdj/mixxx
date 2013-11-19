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

    inline const QColor& getSignalColor() const { return m_signalColor;}
    inline const QColor& getLowColor() const { return m_lowColor;}
    inline const QColor& getMidColor() const { return m_midColor;}
    inline const QColor& getHighColor() const { return m_highColor;}
    inline const QColor& getAxesColor() const { return m_axesColor;}
    inline const QColor& getPlayPosColor() const { return m_playPosColor;}
    inline const QColor& getBgColor() const { return m_bgColor;}

protected:
    void fallBackFromSignalColor();
    void fallBackDefaultColor();

    float stableHue( float hue) const;

private:
    QColor m_signalColor;
    QColor m_lowColor;
    QColor m_midColor;
    QColor m_highColor;
    QColor m_axesColor;
    QColor m_playPosColor;
    QColor m_bgColor;
};

#endif // WAVEFORMSIGNALCOLORS_H
