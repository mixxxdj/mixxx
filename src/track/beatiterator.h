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
              m_endBeat(end) {
    }

    using iterator_category = std::forward_iterator_tag;

    bool hasNext() const {
        return m_currentBeat != m_endBeat;
    }

    /// Advances the iterator and returns the current beat frame position.
    /// If you need the frame position make sure you store it, is not possible
    /// to get it again.
    track::io::Beat next() {
        if (hasNext()) {
            return *m_currentBeat++;
        } else {
            return track::io::Beat();
        }
    }

    /*
    // TODO(hacksdump): These will be removed from this iterator and
    // will be made methods of the Beat class to be introduced in #2844.

    /// Returns true if the current beat is a down beat.
    bool isBar() const {
        return m_currentBeat->type() == mixxx::track::io::BAR;
    }

    /// Returns true if the current beat is a phrase beat.
    bool isPhrase() const {
        return m_currentBeat->type() == mixxx::track::io::PHRASE;
    }

    /// The current beat becomes a regular beat.
    void makeBeat() {
        // TODO(JVC) Const_cast is needed until we manage to make BeatIterator read/write
        const_cast<mixxx::track::io::Beat&>(*m_currentBeat).set_type(mixxx::track::io::BEAT);
    }

    /// The current beat becomes a downbeat.
    void makeBar() {
        // TODO(JVC) Const_cast is needed until we manage to make BeatIterator read/write
        const_cast<mixxx::track::io::Beat&>(*m_currentBeat).set_type(mixxx::track::io::BAR);
    }*/

  private:
    BeatList::const_iterator m_currentBeat;
    BeatList::const_iterator m_endBeat;
};

} // Namespace mixxx
