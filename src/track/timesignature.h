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
    inline TimeSignature(int beatsPerBar, int noteValue)
        : m_beatsPerBar(beatsPerBar),
          m_noteValue(noteValue) {
    }

    ~TimeSignature() {
    }

    inline void setBeatsPerBar(int beatsPerBar) {
        m_beatsPerBar = beatsPerBar;
    }

    inline void setNoteValue(int noteValue) {
        m_noteValue = noteValue;
    }

    inline void setTimeSignature(int beatsPerBar, int noteValue) {
        m_beatsPerBar = beatsPerBar;
        m_noteValue = noteValue;
    }

    inline int getBeatsPerBar() const {
        return m_beatsPerBar;
    }

    inline int getNoteValue() const {
        return m_noteValue;
    }

    inline bool operator==(const TimeSignature& signature) const {
        return(signature.m_beatsPerBar == m_beatsPerBar && signature.m_noteValue == m_noteValue);
    }

    inline bool operator!=(const TimeSignature& signature) const {
        return (signature.m_beatsPerBar != m_beatsPerBar || signature.m_noteValue != m_noteValue);
    }
};

// Invalid Signature
const TimeSignature kNullTimeSignature = TimeSignature(0,0);
// Default Signature 4/4
const TimeSignature kDefaultTimeSignature = TimeSignature(4,4);

} // namespace mixxx
