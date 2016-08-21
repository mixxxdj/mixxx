#ifndef WAVEFORMMARKSET_H
#define WAVEFORMMARKSET_H

#include <QList>
#include <QSharedPointer>

#include "waveformmark.h"
#include "skin/skincontext.h"
#include "util/memory.h"

class WaveformWidgetRenderer;

class WaveformMarkSet {
  public:
    WaveformMarkSet();
    virtual ~WaveformMarkSet();

    void setup(const QString& group, const QDomNode& node,
               const SkinContext& context,
               const WaveformSignalColors& signalColors);
    void clear();

    int size() const { return m_marks.size();}
    QSharedPointer<WaveformMark> operator[] (int i);

    // hotCue must be valid (>= 0 and < NUM_HOT_CUES)
    QSharedPointer<WaveformMark> getHotCueMark(int hotCue);
    void setHotCueMark(int hotCue, QSharedPointer<WaveformMark> pMark);

  private:
    WaveformMark m_defaultMark;
    QList<QSharedPointer<WaveformMark>> m_marks;
    int m_iFirstHotCue;
    DISALLOW_COPY_AND_ASSIGN(WaveformMarkSet);
};

#endif // WAVEFORMMARKSET_H
