#ifndef WAVEFORMMARK_H
#define WAVEFORMMARK_H

#include <QString>
#include <QPixmap>

class ControlObject;

class WaveformMark
{
public:
    WaveformMark();

private:
    ControlObject* m_point;
    QColor m_color;
    QColor m_textColor;
    QString m_text;
    Qt::Alignment m_align;
    QString m_pixmapPath;
    QPixmap m_pixmap;

    friend class WaveformRenderMark;
};

#endif // WAVEFORMMARK_H
