// signature.h
// Created on: 06/11/2019 by Javier Vilarroig

#pragma once

namespace mixxx {

// Musical Time signature
// Right now is to be used for bar detection only, so only the beats per bar is useful
class TimeSignature final {
  private:
    int m_beats;
    int m_noteValue;

  public:
    inline TimeSignature(int beats, int noteValue) {
        m_beats = beats;
        m_noteValue = noteValue;
    }

    virtual ~TimeSignature() {}

    inline void setBeats(int beats) {
        m_beats = beats;
    }

    inline void setNoteValue(int noteValue) {
        m_noteValue = noteValue;
    }

    inline int getBeats() {
        return m_beats;
    }

    inline int getNoteValue() {
        return m_noteValue;
    }

    inline bool operator==(const TimeSignature& signature) const {
        return(signature.m_beats == m_beats && signature.m_noteValue == m_noteValue);
    }
};

// Invalid Signature
const TimeSignature null_time_signature = TimeSignature(0,0);
// Default Signature 4/4
const TimeSignature default_time_signature = TimeSignature(4,4);

} // namespace mixxx
