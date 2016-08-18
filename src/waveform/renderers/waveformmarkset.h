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
    void clear();

    int size() const { return m_marks.size();}
    WaveformMark* operator[] (int i) { return m_marks[i]; }

    const WaveformMark* getDefaultMark() const { return m_pDefaultMark.get();}

    // hotCue must be valid (>= 0 and < NUM_HOT_CUES)
    const WaveformMark* getHotCueMark(int hotCue);
    // Takes ownership of mark
    void setHotCueMark(int hotCue, WaveformMark* mark);

  private:
    std::unique_ptr<WaveformMark> m_pDefaultMark;
    QList<WaveformMark*> m_marks;
    int m_iFirstHotCue;
    DISALLOW_COPY_AND_ASSIGN(WaveformMarkSet);
};

#endif // WAVEFORMMARKSET_H
