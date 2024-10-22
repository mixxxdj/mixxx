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

QmlWaveformRendererBeat::QmlWaveformRendererBeat() {
}

QmlWaveformRendererMarkRange::QmlWaveformRendererMarkRange() {
}

QmlWaveformRendererStem::QmlWaveformRendererStem() {
}

QmlWaveformRendererMark::QmlWaveformRendererMark()
        : m_defaultMark(nullptr),
          m_untilMark(std::make_unique<QmlWaveformUntilMark>()) {
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererEndOfTrack::create(
        WaveformWidgetRenderer* waveformWidget) const {
    auto* renderer = new WaveformRendererEndOfTrack(waveformWidget, m_color);
    return QmlWaveformRendererFactory::Renderer{renderer, renderer};
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererPreroll::create(
        WaveformWidgetRenderer* waveformWidget) const {
    auto* renderer = new WaveformRendererPreroll(
            waveformWidget, WaveformRendererAbstract::Play, m_color);
    return QmlWaveformRendererFactory::Renderer{renderer, renderer};
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererRGB::create(
        WaveformWidgetRenderer* waveformWidget) const {
    auto* renderer = new WaveformRendererRGB(waveformWidget,
            m_axesColor,
            m_lowColor,
            m_midColor,
            m_highColor,
            ::WaveformRendererAbstract::Play);
    return QmlWaveformRendererFactory::Renderer{renderer, renderer};
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererBeat::create(
        WaveformWidgetRenderer* waveformWidget) const {
    auto* renderer = new WaveformRenderBeat(
            waveformWidget, ::WaveformRendererAbstract::Play, m_color);
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
            waveformWidget, ::WaveformRendererAbstract::Play);
    return QmlWaveformRendererFactory::Renderer{renderer, renderer};
}
#endif

QmlWaveformRendererFactory::Renderer QmlWaveformRendererMark::create(
        WaveformWidgetRenderer* waveformWidget) const {
    VERIFY_OR_DEBUG_ASSERT(!!m_untilMark) {
        return QmlWaveformRendererFactory::Renderer{};
    }
    auto* renderer = new WaveformRenderMark(waveformWidget,
            m_playMarkerColor,
            m_playMarkerBackground,
            m_untilMark->showTime(),
            m_untilMark->showBeats(),
            static_cast<Qt::Alignment>(m_untilMark->align()),
            m_untilMark->textSize(),
            ::WaveformRendererAbstract::Play);
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
