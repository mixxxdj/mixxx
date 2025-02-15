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
    auto* renderer = new WaveformRendererEndOfTrack(waveformWidget);

    renderer->setColor(m_color);
    renderer->setEndOfTrackWarningTime(m_endOfTrackWarningTime);
    connect(this,
            &QmlWaveformRendererEndOfTrack::colorChanged,
            renderer,
            &WaveformRendererEndOfTrack::setColor);
    connect(this,
            &QmlWaveformRendererEndOfTrack::endOfTrackWarningTimeChanged,
            renderer,
            &WaveformRendererEndOfTrack::setEndOfTrackWarningTime);
    return QmlWaveformRendererFactory::Renderer{renderer, renderer};
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererPreroll::create(
        WaveformWidgetRenderer* waveformWidget) const {
    auto* renderer = new WaveformRendererPreroll(
            waveformWidget, m_position);
    renderer->setColor(m_color);
    connect(this,
            &QmlWaveformRendererPreroll::colorChanged,
            renderer,
            &WaveformRendererPreroll::setColor);
    return QmlWaveformRendererFactory::Renderer{renderer, renderer};
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererRGB::create(
        WaveformWidgetRenderer* waveformWidget) const {
    auto* renderer = new WaveformRendererRGB(waveformWidget, m_position, m_options);

    renderer->setAxesColor(m_axesColor);
    renderer->setLowColor(m_lowColor);
    renderer->setMidColor(m_midColor);
    renderer->setHighColor(m_highColor);
    connect(this,
            &QmlWaveformRendererRGB::axesColorChanged,
            renderer,
            &WaveformRendererRGB::setAxesColor);
    connect(this,
            &QmlWaveformRendererRGB::lowColorChanged,
            renderer,
            &WaveformRendererRGB::setLowColor);
    connect(this,
            &QmlWaveformRendererRGB::midColorChanged,
            renderer,
            &WaveformRendererRGB::setMidColor);
    connect(this,
            &QmlWaveformRendererRGB::highColorChanged,
            renderer,
            &WaveformRendererRGB::setHighColor);

    renderer->setAllChannelVisualGain(m_gainAll);
    renderer->setLowVisualGain(m_gainLow);
    renderer->setMidVisualGain(m_gainMid);
    renderer->setHighVisualGain(m_gainHigh);
    connect(this,
            &QmlWaveformRendererRGB::gainAllChanged,
            renderer,
            &WaveformRendererRGB::setAllChannelVisualGain);
    connect(this,
            &QmlWaveformRendererRGB::gainLowChanged,
            renderer,
            &WaveformRendererRGB::setLowVisualGain);
    connect(this,
            &QmlWaveformRendererRGB::gainMidChanged,
            renderer,
            &WaveformRendererRGB::setMidVisualGain);
    connect(this,
            &QmlWaveformRendererRGB::gainHighChanged,
            renderer,
            &WaveformRendererRGB::setHighVisualGain);
    return QmlWaveformRendererFactory::Renderer{renderer, renderer};
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererBeat::create(
        WaveformWidgetRenderer* waveformWidget) const {
    auto* renderer = new WaveformRenderBeat(
            waveformWidget, m_position);
    renderer->setColor(m_color);
    connect(this, &QmlWaveformRendererBeat::colorChanged, renderer, &WaveformRenderBeat::setColor);
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
            waveformWidget, m_position);

    renderer->setAllChannelVisualGain(m_gainAll);
    connect(this,
            &QmlWaveformRendererStem::gainAllChanged,
            renderer,
            &WaveformRendererStem::setAllChannelVisualGain);
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

    renderer->setFgPlayColor(m_playMarkerColor);
    renderer->setBgPlayColor(m_playMarkerBackground);

    renderer->setUntilMarkShowBeats(m_untilMark->showTime());
    renderer->setUntilMarkShowTime(m_untilMark->showBeats());
    renderer->setUntilMarkAlign(static_cast<Qt::Alignment>(m_untilMark->align()));
    renderer->setUntilMarkTextSize(m_untilMark->textSize());
    renderer->setUntilMarkTextHeightLimit(m_untilMark->textHeightLimit());

    connect(m_untilMark.get(),
            &QmlWaveformUntilMark::showTimeChanged,
            renderer,
            &WaveformRenderMark::setUntilMarkShowTime);
    connect(m_untilMark.get(),
            &QmlWaveformUntilMark::showBeatsChanged,
            renderer,
            &WaveformRenderMark::setUntilMarkShowBeats);
    connect(m_untilMark.get(),
            &QmlWaveformUntilMark::alignChanged,
            renderer,
            &WaveformRenderMark::setUntilMarkAlign);
    connect(m_untilMark.get(),
            &QmlWaveformUntilMark::textSizeChanged,
            renderer,
            &WaveformRenderMark::setUntilMarkTextSize);

    connect(this,
            &QmlWaveformRendererMark::playMarkerColorChanged,
            renderer,
            &WaveformRenderMark::setFgPlayColor);
    connect(this,
            &QmlWaveformRendererMark::playMarkerBackgroundChanged,
            renderer,
            &WaveformRenderMark::setBgPlayColor);

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
