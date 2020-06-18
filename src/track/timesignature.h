// signature.h
// Created on: 06/11/2019 by Javier Vilarroig

#pragma once

namespace mixxx {

/// Musical Time signature
/// Right now is to be used for bar detection only, so only the beats per bar is useful
class TimeSignature final {
  private:
    int m_beatsPerBar;
    int m_noteValue;

  public:
    TimeSignature(int beatsPerBar, int noteValue)
            : m_beatsPerBar(beatsPerBar),
              m_noteValue(noteValue) {
    }

    ~TimeSignature() {
    }

    void setBeatsPerBar(int beatsPerBar) {
        m_beatsPerBar = beatsPerBar;
    }

    void setNoteValue(int noteValue) {
        m_noteValue = noteValue;
    }

    void setTimeSignature(int beatsPerBar, int noteValue) {
        m_beatsPerBar = beatsPerBar;
        m_noteValue = noteValue;
    }

    int getBeatsPerBar() const {
        return m_beatsPerBar;
    }

    int getNoteValue() const {
        return m_noteValue;
    }
};

inline bool operator==(TimeSignature signature1, TimeSignature signature2) {
    return signature1.getBeatsPerBar() == signature2.getBeatsPerBar() &&
            signature1.getNoteValue() == signature2.getNoteValue();
}

inline bool operator!=(TimeSignature signature1, TimeSignature signature2) {
    return !(signature1 == signature2);
}

// Invalid Signature
const TimeSignature kNullTimeSignature = TimeSignature(0,0);
// Default Signature 4/4
const TimeSignature kDefaultTimeSignature = TimeSignature(4,4);

} // namespace mixxx
