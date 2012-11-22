#ifndef WAVEFORMMARK_H
#define WAVEFORMMARK_H

#include <QString>
#include <QPixmap>

class ControlObjectThreadMain;
class QDomNode;

class WaveformMark {
  public:
    WaveformMark();
    void setup( const QString& group, const QDomNode& node);

  private:
    ControlObjectThreadMain* m_pointControl;

    QColor m_color;
    QColor m_textColor;
    QString m_text;
    Qt::Alignment m_align;
    QString m_pixmapPath;
    QPixmap m_pixmap;

    friend class WaveformMarkSet;
    friend class WaveformRenderMark;
    friend class WOverview;
};

#endif // WAVEFORMMARK_H
