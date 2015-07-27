#ifndef WAVEFORMMARKSET_H
#define WAVEFORMMARKSET_H

#include <QList>

#include "waveformmark.h"
#include "skin/skincontext.h"

class WaveformWidgetRenderer;

class WaveformMarkSet
{
public:
    WaveformMarkSet();
    ~WaveformMarkSet();
    void setup(const QString& group, const QDomNode& node,
               const SkinContext& context,
               const WaveformSignalColors& signalColors);
    void clear();

    int size() const { return m_marks.size();}
    WaveformMark& operator[] (int i) { return m_marks[i];}

    const WaveformMark& getDefaultMark() const { return m_defaultMark;}
    const QList<WaveformMark>& getMarks() const { return m_marks;}

private:
    WaveformMark m_defaultMark;
    QList<WaveformMark> m_marks;
};

#endif // WAVEFORMMARKSET_H
