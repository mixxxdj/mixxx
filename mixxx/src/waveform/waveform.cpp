#include "waveform/waveform.h"

#include <cstring>

Waveform::Waveform()
        : m_visualSampleRate(441) {
}

Waveform::~Waveform() {
}

void Waveform::resize(int size) {
    m_data.resize(size);
}

void Waveform::reset(int value) {
    m_data.assign(m_data.size(),value);
}

void Waveform::assign(int size, int value) {
    m_data.assign(size,value);
}
