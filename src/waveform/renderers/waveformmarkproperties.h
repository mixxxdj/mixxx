#ifndef WAVEFORMMARKPROPERTIES_H
#define WAVEFORMMARKPROPERTIES_H

#include <QColor>
#include <QDomNode>

class SkinContext;
class WaveformSignalColors;

class WaveformMarkProperties final {
  public:
    WaveformMarkProperties() = default;
    WaveformMarkProperties(const QDomNode& node,
                           const SkinContext& context,
                           const WaveformSignalColors& signalColors);
    void setHotCueNumber(int i) { m_text = m_text.arg(i); }

    QColor m_color;
    QColor m_textColor;
    QString m_text;
    Qt::Alignment m_align;
    QString m_pixmapPath;
};

#endif // WAVEFORMMARKPROPERTIES_H
