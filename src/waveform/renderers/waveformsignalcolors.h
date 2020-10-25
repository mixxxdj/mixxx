#ifndef WAVEFORMSIGNALCOLORS_H
#define WAVEFORMSIGNALCOLORS_H

#include <QColor>
#include <QDomNode>

#include "skin/skincontext.h"

class WaveformSignalColors {
  public:
    WaveformSignalColors();
    virtual ~WaveformSignalColors() {}

    bool setup(const QDomNode &node, const SkinContext& context);

    inline const QColor& getSignalColor() const {
        return m_signalColor;
    }
    inline const QColor& getLowColor() const {
        return m_lowColor;
    }
    inline const QColor& getMidColor() const {
        return m_midColor;
    }
    inline const QColor& getHighColor() const {
        return m_highColor;
    }
    inline const QColor& getRgbLowColor() const {
        return m_rgbLowColor;
    }
    inline const QColor& getRgbMidColor() const {
        return m_rgbMidColor;
    }
    inline const QColor& getRgbHighColor() const {
        return m_rgbHighColor;
    }
    inline const QColor& getAxesColor() const {
        return m_axesColor;
    }
    inline const QColor& getPlayPosColor() const {
        return m_playPosColor;
    }
    inline const QColor& getPlayedOverlayColor() const {
        return m_playedOverlayColor;
    }
    inline const QColor& getPassthroughOverlayColor() const {
        return m_passthroughOverlayColor;
    }
    inline const QColor& getBgColor() const {
        return m_bgColor;
    }
    inline int getDimBrightThreshold() const {
        return m_dimBrightThreshold;
    }

  protected:
    void fallBackFromSignalColor();
    void fallBackDefaultColor();

    double stableHue(double hue) const;

  private:
    QColor m_signalColor;
    QColor m_lowColor;
    QColor m_midColor;
    QColor m_highColor;
    QColor m_rgbLowColor;
    QColor m_rgbMidColor;
    QColor m_rgbHighColor;
    QColor m_axesColor;
    QColor m_playPosColor;
    QColor m_playedOverlayColor;
    QColor m_passthroughOverlayColor;
    QColor m_bgColor;
    int m_dimBrightThreshold;
};

#endif // WAVEFORMSIGNALCOLORS_H
