#ifndef WAVEFORMMARKSET_H
#define WAVEFORMMARKSET_H

#include "waveformmark.h"
#include <QList>

class WaveformWidgetRenderer;

class WaveformMarkSet
{
public:
    WaveformMarkSet();
    void setup(const QString& group, const QDomNode& node,
            const WaveformSignalColors& signalColors);

    int size() const { return m_marks.size();}
    WaveformMark& operator[] (int i) { return m_marks[i];}

    const WaveformMark& getDefaultMark() const { return m_defaultMark;}
    const QList<WaveformMark>& getMarks() const { return m_marks;}

private:
    WaveformMark m_defaultMark;
    QList<WaveformMark> m_marks;
};

#endif // WAVEFORMMARKSET_H
