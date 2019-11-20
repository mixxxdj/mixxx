// signature.h
// Created on: 06/11/20196 nov. 2019 by Javier Vilarroig

#pragma once

namespace mixxx {

// Musical Time signature
// Right now is to be used for bar detection, so only the beats per bar is useful
class Signature final {
  private:
    int m_beats;
    int m_noteValue;

  public:
    Signature(int a, int b) {
        m_beats = a;
        m_noteValue = b;
    }

    virtual ~Signature() {}

    void setBeats(int a) {
        m_beats = a;
    }

    void setNoteValue(int a) {
        m_noteValue = a;
    }

    int getBeats() {
        return m_beats;
    }

    int getNoteValue() {
        return m_noteValue;
    }
};

}
