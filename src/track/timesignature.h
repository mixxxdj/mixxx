#pragma once

#include "proto/beats.pb.h"

namespace mixxx {

/// Musical Time signature
/// Right now is to be used for bar detection only, so only the beats per bar is useful
class TimeSignature final {
  public:
    // The default constructor will set time signature to 4/4
    // since it is defined as the default in beats.proto
    TimeSignature() = default;

    TimeSignature(int beatsPerBar, int noteValue) {
        setTimeSignature(beatsPerBar, noteValue);
    }

    explicit TimeSignature(track::io::TimeSignature timeSignatureProto) {
        m_timeSignature = timeSignatureProto;
    }

    ~TimeSignature() = default;

    void setBeatsPerBar(int beatsPerBar) {
        m_timeSignature.set_beats_per_bar(beatsPerBar);
    }

    void setNoteValue(int noteValue) {
        m_timeSignature.set_note_value(noteValue);
    }

    void setTimeSignature(int beatsPerBar, int noteValue) {
        setBeatsPerBar(beatsPerBar);
        setNoteValue(noteValue);
    }

    int getBeatsPerBar() const {
        return m_timeSignature.beats_per_bar();
    }

    int getNoteValue() const {
        return m_timeSignature.note_value();
    }

  private:
    // Using the protocol buffer defined type.
    track::io::TimeSignature m_timeSignature;
};

inline bool operator==(TimeSignature signature1, TimeSignature signature2) {
    return signature1.getBeatsPerBar() == signature2.getBeatsPerBar() &&
            signature1.getNoteValue() == signature2.getNoteValue();
}

inline bool operator!=(TimeSignature signature1, TimeSignature signature2) {
    return !(signature1 == signature2);
}

inline QDebug operator<<(QDebug dbg, TimeSignature timeSignature) {
    return dbg << timeSignature.getBeatsPerBar() << "/" << timeSignature.getNoteValue();
}

// Invalid Signature
const TimeSignature kNullTimeSignature = TimeSignature(0,0);

} // namespace mixxx
