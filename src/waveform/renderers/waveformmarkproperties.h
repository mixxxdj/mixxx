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
                           const WaveformSignalColors& signalColors,
                           int hotCue);

    // Sets the appropriate mark colors based on the base color
    void setBaseColor(QColor baseColor);
    QColor fillColor() const;
    QColor borderColor() const;
    QColor labelColor() const;

    QColor m_textColor;
    QString m_text;
    Qt::Alignment m_align;
    QString m_pixmapPath;

  private:
    QColor m_fillColor;
    QColor m_borderColor;
    QColor m_labelColor;
};

#endif // WAVEFORMMARKPROPERTIES_H
