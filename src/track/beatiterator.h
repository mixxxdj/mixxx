#pragma once

#include "track/beats.h"
#include "track/beatutils.h"

namespace mixxx {
/// A forward iterator for Beats.
/// BeatIterator do not fulfills the Iterator semantics, use with care.
class BeatIterator final {
  public:
    BeatIterator(BeatList::const_iterator start, BeatList::const_iterator end)
            : m_currentBeat(start),
              m_endBeat(end),
              m_ended(false) {
        // Advance to the first enabled beat.
        while (m_currentBeat <= m_endBeat && !m_currentBeat->enabled()) {
            ++m_currentBeat;
        }
    }

    ~BeatIterator() = default;

    bool hasNext() const {
        return !m_ended;
    }

    /// Advances de iterator and returns the current beat frame position.
    /// If you need the frame position make sure you store it, is not possible
    /// to get it again.
    double next() {
        double beat = m_currentBeat->frame_position();
        if (beat != m_endBeat->frame_position()) {
            ++m_currentBeat;
            while (beat != m_endBeat->frame_position() && !m_currentBeat->enabled()) {
                ++m_currentBeat;
            }
        } else {
            m_ended = true;
        }
        return beat;
    }

    /// Returns true if the current beat is a down beat.
    bool isBar() const {
        return m_currentBeat->type() == mixxx::track::io::BAR;
    }

    /// Returns true if the current beat is a phrase beat.
    bool isPhrase() const {
        return m_currentBeat->type() == mixxx::track::io::PHRASE;
    }

    /// The current beat becomes a down beat.
    void makeBeat() {
        // TODO(JVC) Const_cast is needed until we manage to make BeatIterator read/write
        const_cast<mixxx::track::io::Beat&>(*m_currentBeat).set_type(mixxx::track::io::BEAT);
    }

    /// The current beat becomes a phrase beat.
    void makeBar() {
        // TODO(JVC) Const_cast is needed until we manage to make BeatIterator read/write
        const_cast<mixxx::track::io::Beat&>(*m_currentBeat).set_type(mixxx::track::io::BAR);
    }

  private:
    BeatList::const_iterator m_currentBeat;
    BeatList::const_iterator m_endBeat;
    bool m_ended;
};

} // Namespace mixxx
