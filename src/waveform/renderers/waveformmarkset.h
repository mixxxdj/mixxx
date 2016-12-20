#ifndef WAVEFORMMARKSET_H
#define WAVEFORMMARKSET_H

#include <QList>

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

    int size() const { return m_marks.size();}
    WaveformMarkPointer operator[] (int i) const { return m_marks[i]; };

    // hotCue must be valid (>= 0 and < NUM_HOT_CUES)
    WaveformMarkPointer getHotCueMark(int hotCue) const;
    void setHotCueMark(int hotCue, WaveformMarkPointer pMark);

  private:
    void clear(){ m_marks.clear(); }
    std::unique_ptr<WaveformMark> m_pDefaultMark;
    QList<WaveformMarkPointer> m_marks;
    int m_iFirstHotCue;
    DISALLOW_COPY_AND_ASSIGN(WaveformMarkSet);
};

#endif // WAVEFORMMARKSET_H
