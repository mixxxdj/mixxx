#pragma once

#include <QList>

#include "waveformmark.h"
#include "skin/skincontext.h"
#include "util/memory.h"

class WaveformWidgetRenderer;

// This class helps share code between the WaveformRenderMark and WOverview
// constructors.
class WaveformMarkSet {
  public:
    WaveformMarkSet();
    virtual ~WaveformMarkSet();

    void setup(const QString& group, const QDomNode& node,
               const SkinContext& context,
               const WaveformSignalColors& signalColors);

    inline QList<WaveformMarkPointer>::const_iterator begin() const { return m_marks.begin(); }
    inline QList<WaveformMarkPointer>::const_iterator end() const { return m_marks.end(); }

    // hotCue must be valid (>= 0 and < NUM_HOT_CUES)
    WaveformMarkPointer getHotCueMark(int hotCue) const;
    WaveformMarkPointer getDefaultMark() const;

  private:
    void clear() { m_marks.clear(); }
    WaveformMarkPointer m_pDefaultMark;
    QList<WaveformMarkPointer> m_marks;
    QMap<int, WaveformMarkPointer> m_hotCueMarks;

    DISALLOW_COPY_AND_ASSIGN(WaveformMarkSet);
};
