#pragma once

#include "track/beats.h"
#include "track/beatutils.h"

namespace mixxx {
class BeatIterator final {
  public:
    BeatIterator(BeatList::const_iterator start, BeatList::const_iterator end)
            : m_currentBeat(start),
              m_endBeat(end) {
        // Advance to the first enabled beat.
        while (m_currentBeat != m_endBeat && !m_currentBeat->enabled()) {
            ++m_currentBeat;
        }
    }

    ~BeatIterator() = default;

    bool hasNext() const {
        return m_currentBeat != m_endBeat;
    }

    double next() {
        double beat = BeatUtils::framesToSamples(m_currentBeat->frame_position());
        ++m_currentBeat;
        while (m_currentBeat != m_endBeat && !m_currentBeat->enabled()) {
            ++m_currentBeat;
        }
        return beat;
    }

    bool isBar() const {
        return m_currentBeat->type() == mixxx::track::io::BAR;
    }

    bool isPhrase() const {
        return m_currentBeat->type() == mixxx::track::io::PHRASE;
    }

    void makeBeat() {
        const_cast<mixxx::track::io::Beat&>(*m_currentBeat).set_type(mixxx::track::io::BEAT);
    }

    void makeBar() {
        const_cast<mixxx::track::io::Beat&>(*m_currentBeat).set_type(mixxx::track::io::BAR);
    }

  private:
    BeatList::const_iterator m_currentBeat;
    BeatList::const_iterator m_endBeat;
};

} // Namespace mixxx
