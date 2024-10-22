#include "qml/qmlwaveformrenderer.h"

#include <memory>

#include "moc_qmlwaveformrenderer.cpp"
#include "util/assert.h"
#include "waveform/renderers/allshader/waveformrenderbeat.h"
#include "waveform/renderers/allshader/waveformrendererendoftrack.h"
#include "waveform/renderers/allshader/waveformrendererpreroll.h"
#include "waveform/renderers/allshader/waveformrendererrgb.h"
#ifdef __STEM__
#include "waveform/renderers/allshader/waveformrendererstem.h"
#endif
#include "waveform/renderers/allshader/waveformrendermark.h"
#include "waveform/renderers/allshader/waveformrendermarkrange.h"

using namespace allshader;

namespace mixxx {
namespace qml {

QmlWaveformRendererEndOfTrack::QmlWaveformRendererEndOfTrack() {
}

QmlWaveformRendererPreroll::QmlWaveformRendererPreroll() {
}

QmlWaveformRendererRGB::QmlWaveformRendererRGB() {
}

double QmlWaveformRendererRGB::getVisualGain(FilterIndex index) const {
    switch (index) {
    case AllChannel:
        return m_gainAll;
    case Low:
        return m_gainLow;
    case Mid:
        return m_gainMid;
    case High:
        return m_gainHigh;
    case FilterCount:
        break;
    }
    return 1.0;
}

QmlWaveformRendererBeat::QmlWaveformRendererBeat() {
}

QmlWaveformRendererMarkRange::QmlWaveformRendererMarkRange() {
}

QmlWaveformRendererStem::QmlWaveformRendererStem() {
}

double QmlWaveformRendererStem::getVisualGain(FilterIndex index) const {
    switch (index) {
    case AllChannel:
        return m_gainAll;
    case Low:
    case Mid:
    case High:
    case FilterCount:
        break;
    }
    return 1.0;
}

QmlWaveformRendererMark::QmlWaveformRendererMark()
        : m_defaultMark(nullptr),
          m_untilMark(std::make_unique<QmlWaveformUntilMark>()) {
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererEndOfTrack::create(
        WaveformWidgetRenderer* waveformWidget) const {
    auto* renderer = new WaveformRendererEndOfTrack(waveformWidget);
    renderer->setup(m_color, m_endOfTrackWarningTime);
    return QmlWaveformRendererFactory::Renderer{renderer, renderer};
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererPreroll::create(
        WaveformWidgetRenderer* waveformWidget) const {
    auto* renderer = new WaveformRendererPreroll(
            waveformWidget, m_position);
    renderer->setup(m_color);
    return QmlWaveformRendererFactory::Renderer{renderer, renderer};
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererRGB::create(
        WaveformWidgetRenderer* waveformWidget) const {
    auto* renderer = new WaveformRendererRGB(waveformWidget, m_position, m_options, this);
    renderer->setup(
            m_axesColor,
            m_lowColor,
            m_midColor,
            m_highColor);
    return QmlWaveformRendererFactory::Renderer{renderer, renderer};
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererBeat::create(
        WaveformWidgetRenderer* waveformWidget) const {
    auto* renderer = new WaveformRenderBeat(
            waveformWidget, m_position);
    renderer->setup(m_color);
    return QmlWaveformRendererFactory::Renderer{renderer, renderer};
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererMarkRange::create(
        WaveformWidgetRenderer* waveformWidget) const {
    auto* renderer = new WaveformRenderMarkRange(
            waveformWidget);

    for (auto* pMark : m_ranges) {
        renderer->addRange(WaveformMarkRange(
                waveformWidget->getGroup(),
                pMark->color(),
                pMark->disabledColor(),
                pMark->opacity(),
                pMark->disabledOpacity(),
                pMark->durationTextColor(),
                pMark->startControl(),
                pMark->endControl(),
                pMark->enabledControl(),
                pMark->visibilityControl(),
                pMark->durationTextLocation()));
    }
    return QmlWaveformRendererFactory::Renderer{renderer, renderer};
}

#ifdef __STEM__
QmlWaveformRendererFactory::Renderer QmlWaveformRendererStem::create(
        WaveformWidgetRenderer* waveformWidget) const {
    auto* renderer = new WaveformRendererStem(
            waveformWidget, m_position, this);
    return QmlWaveformRendererFactory::Renderer{renderer, renderer};
}
#endif

QmlWaveformRendererFactory::Renderer QmlWaveformRendererMark::create(
        WaveformWidgetRenderer* waveformWidget) const {
    VERIFY_OR_DEBUG_ASSERT(!!m_untilMark) {
        return QmlWaveformRendererFactory::Renderer{};
    }
    auto* renderer = new WaveformRenderMark(waveformWidget,
            ::WaveformRendererAbstract::Play);
    renderer->setup(
            m_playMarkerColor,
            m_playMarkerBackground,
            m_untilMark->showTime(),
            m_untilMark->showBeats(),
            static_cast<Qt::Alignment>(m_untilMark->align()),
            m_untilMark->textSize(),
            m_untilMark->textHeightLimit());
    int priority = 0;
    for (auto* pMark : m_marks) {
        renderer->addMark(WaveformMarkPointer(new WaveformMark(
                waveformWidget->getGroup(),
                pMark->control(),
                pMark->visibilityControl(),
                pMark->textColor(),
                pMark->align(),
                pMark->text(),
                pMark->pixmap(),
                pMark->icon(),
                pMark->color(),
                --priority)));
    }
    auto* pMark = defaultMark();
    if (pMark != nullptr) {
        renderer->setDefaultMark(
                waveformWidget->getGroup(),
                WaveformMarkSet::Seed{
                        pMark->control(),
                        pMark->visibilityControl(),
                        pMark->textColor(),
                        pMark->align(),
                        pMark->text(),
                        pMark->pixmap(),
                        pMark->icon(),
                        pMark->color(),
                });
    }
    return QmlWaveformRendererFactory::Renderer{renderer, renderer};
}
} // namespace qml
} // namespace mixxx
