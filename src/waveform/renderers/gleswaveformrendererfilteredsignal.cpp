#include "glwaveformrendererfilteredsignal.h"
#include "trackinfoobject.h"
#include "waveform/waveform.h"
#include "waveformwidgetrenderer.h"
#include "controlobjectthread.h"
#include "waveform/waveformwidgetfactory.h"
#include "util/math.h"

#include <QDomNode>

#include <qgl.h>

GLWaveformRendererFilteredSignal::GLWaveformRendererFilteredSignal(
        WaveformWidgetRenderer* waveformWidgetRenderer)
    : WaveformRendererSignalBase(waveformWidgetRenderer) {

}

GLWaveformRendererFilteredSignal::~GLWaveformRendererFilteredSignal() {

}

void GLWaveformRendererFilteredSignal::onSetup(const QDomNode& /*node*/) {

}

void GLWaveformRendererFilteredSignal::draw(QPainter* painter, QPaintEvent* /*event*/) {

    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack) {
        return;
    }

    ConstWaveformPointer waveform = pTrack->getWaveform();
    if (waveform.isNull()) {
        return;
    }

    const int dataSize = waveform->getDataSize();
    if (dataSize <= 1) {
        return;
    }

    const WaveformData* data = waveform->data();
    if (data == NULL) {
        return;
    }

    double firstVisualIndex = m_waveformRenderer->getFirstDisplayedPosition() * dataSize;
    double lastVisualIndex = m_waveformRenderer->getLastDisplayedPosition() * dataSize;

    const int firstIndex = int(firstVisualIndex+0.5);
    firstVisualIndex = firstIndex - firstIndex%2;

    const int lastIndex = int(lastVisualIndex+0.5);
    lastVisualIndex = lastIndex + lastIndex%2;

    // Reset device for native painting
    painter->beginNativePainting();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Per-band gain from the EQ knobs.
    float allGain(1.0), lowGain(1.0), midGain(1.0), highGain(1.0);
    getGains(&allGain, &lowGain, &midGain, &highGain);

    float maxLow[2];
    float maxMid[2];
    float maxHigh[2];
    float meanIndex;

    //  Removed Unsupported OpenGL functions, so that Mixxx can run on OpenGL-ES systems -- amvanbaren 9/2015
    //  TODO(XXX) Rewrite OpenGL code in glwaveformrendererfilteredsignal.cpp to support
    //  the new OpenGL syntax or use Qt instead

    painter->endNativePainting();
}
