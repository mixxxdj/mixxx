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
    Beat next() {
        if (hasNext()) {
            return *m_currentBeat++;
        } else {
            return Beat(mixxx::kInvalidFramePos);
        }
    }

  private:
    BeatList::const_iterator m_currentBeat;
    BeatList::const_iterator m_endBeat;
};

} // Namespace mixxx
