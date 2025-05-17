#include "qml/qmlwaveformrenderer.h"

#include <qapplication.h>
#include <qfileinfo.h>
#include <qglobal.h>
#include <qlist.h>

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

namespace mixxx {
namespace qml {

QmlWaveformRendererMark::QmlWaveformRendererMark()
        : m_defaultMark(nullptr),
          m_untilMark(std::make_unique<QmlWaveformUntilMark>()) {
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererEndOfTrack::create(
        WaveformWidgetRenderer* waveformWidget) const {
    auto pRenderer = std::make_unique<allshader::WaveformRendererEndOfTrack>(waveformWidget);

    pRenderer->setColor(m_color);
    pRenderer->setEndOfTrackWarningTime(m_endOfTrackWarningTime);
    connect(this,
            &QmlWaveformRendererEndOfTrack::colorChanged,
            pRenderer.get(),
            &allshader::WaveformRendererEndOfTrack::setColor);
    connect(this,
            &QmlWaveformRendererEndOfTrack::endOfTrackWarningTimeChanged,
            pRenderer.get(),
            &allshader::WaveformRendererEndOfTrack::setEndOfTrackWarningTime);
    return QmlWaveformRendererFactory::Renderer{pRenderer.get(), std::move(pRenderer)};
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererPreroll::create(
        WaveformWidgetRenderer* waveformWidget) const {
    auto pRenderer = std::make_unique<allshader::WaveformRendererPreroll>(
            waveformWidget, m_position);
    pRenderer->setColor(m_color);
    connect(this,
            &QmlWaveformRendererPreroll::colorChanged,
            pRenderer.get(),
            &allshader::WaveformRendererPreroll::setColor);
    return QmlWaveformRendererFactory::Renderer{pRenderer.get(), std::move(pRenderer)};
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererRGB::create(
        WaveformWidgetRenderer* waveformWidget) const {
    auto pRenderer = std::make_unique<allshader::WaveformRendererRGB>(
            waveformWidget, m_position, m_options);

    pRenderer->setAxesColor(m_axesColor);
    pRenderer->setLowColor(m_lowColor);
    pRenderer->setMidColor(m_midColor);
    pRenderer->setHighColor(m_highColor);
    connect(this,
            &QmlWaveformRendererRGB::axesColorChanged,
            pRenderer.get(),
            &allshader::WaveformRendererRGB::setAxesColor);
    connect(this,
            &QmlWaveformRendererRGB::lowColorChanged,
            pRenderer.get(),
            &allshader::WaveformRendererRGB::setLowColor);
    connect(this,
            &QmlWaveformRendererRGB::midColorChanged,
            pRenderer.get(),
            &allshader::WaveformRendererRGB::setMidColor);
    connect(this,
            &QmlWaveformRendererRGB::highColorChanged,
            pRenderer.get(),
            &allshader::WaveformRendererRGB::setHighColor);

    pRenderer->setAllChannelVisualGain(m_gainAll);
    pRenderer->setLowVisualGain(m_gainLow);
    pRenderer->setMidVisualGain(m_gainMid);
    pRenderer->setHighVisualGain(m_gainHigh);
    connect(this,
            &QmlWaveformRendererRGB::gainAllChanged,
            pRenderer.get(),
            &allshader::WaveformRendererRGB::setAllChannelVisualGain);
    connect(this,
            &QmlWaveformRendererRGB::gainLowChanged,
            pRenderer.get(),
            &allshader::WaveformRendererRGB::setLowVisualGain);
    connect(this,
            &QmlWaveformRendererRGB::gainMidChanged,
            pRenderer.get(),
            &allshader::WaveformRendererRGB::setMidVisualGain);
    connect(this,
            &QmlWaveformRendererRGB::gainHighChanged,
            pRenderer.get(),
            &allshader::WaveformRendererRGB::setHighVisualGain);
    return QmlWaveformRendererFactory::Renderer{pRenderer.get(), std::move(pRenderer)};
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererBeat::create(
        WaveformWidgetRenderer* waveformWidget) const {
    auto pRenderer = std::make_unique<allshader::WaveformRenderBeat>(
            waveformWidget, m_position);
    pRenderer->setColor(m_color);
    connect(this,
            &QmlWaveformRendererBeat::colorChanged,
            pRenderer.get(),
            &allshader::WaveformRenderBeat::setColor);
    return QmlWaveformRendererFactory::Renderer{pRenderer.get(), std::move(pRenderer)};
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererMarkRange::create(
        WaveformWidgetRenderer* waveformWidget) const {
    auto pRenderer = std::make_unique<allshader::WaveformRenderMarkRange>(
            waveformWidget);

    for (auto* pMark : std::as_const(m_ranges)) {
        pRenderer->addRange(WaveformMarkRange(
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
    return QmlWaveformRendererFactory::Renderer{pRenderer.get(), std::move(pRenderer)};
}

#ifdef __STEM__
QmlWaveformRendererFactory::Renderer QmlWaveformRendererStem::create(
        WaveformWidgetRenderer* waveformWidget) const {
    auto pRenderer = std::make_unique<allshader::WaveformRendererStem>(
            waveformWidget, m_position);

    pRenderer->setAllChannelVisualGain(m_gainAll);
    pRenderer->setSplitStemTracks(m_splitStemTracks);
    connect(this,
            &QmlWaveformRendererStem::gainAllChanged,
            pRenderer.get(),
            &allshader::WaveformRendererStem::setAllChannelVisualGain);
    connect(this,
            &QmlWaveformRendererStem::splitStemTracksChanged,
            pRenderer.get(),
            &allshader::WaveformRendererStem::setSplitStemTracks);
    return QmlWaveformRendererFactory::Renderer{pRenderer.get(), std::move(pRenderer)};
}
#endif

QmlWaveformRendererFactory::Renderer QmlWaveformRendererMark::create(
        WaveformWidgetRenderer* waveformWidget) const {
    VERIFY_OR_DEBUG_ASSERT(!!m_untilMark) {
        return QmlWaveformRendererFactory::Renderer{};
    }
    auto pRenderer = std::make_unique<allshader::WaveformRenderMark>(waveformWidget,
            ::WaveformRendererAbstract::Play);

    pRenderer->setPlayMarkerForegroundColor(m_playMarkerColor);
    pRenderer->setPlayMarkerBackgroundColor(m_playMarkerBackground);

    pRenderer->setUntilMarkShowBeats(m_untilMark->showTime());
    pRenderer->setUntilMarkShowTime(m_untilMark->showBeats());
    pRenderer->setUntilMarkAlign(m_untilMark->align());
    pRenderer->setUntilMarkTextSize(m_untilMark->textSize());
    pRenderer->setUntilMarkTextHeightLimit(m_untilMark->textHeightLimit());

    connect(m_untilMark.get(),
            &QmlWaveformUntilMark::showTimeChanged,
            pRenderer.get(),
            &allshader::WaveformRenderMark::setUntilMarkShowTime);
    connect(m_untilMark.get(),
            &QmlWaveformUntilMark::showBeatsChanged,
            pRenderer.get(),
            &allshader::WaveformRenderMark::setUntilMarkShowBeats);
    connect(m_untilMark.get(),
            &QmlWaveformUntilMark::alignChanged,
            pRenderer.get(),
            &allshader::WaveformRenderMark::setUntilMarkAlign);
    connect(m_untilMark.get(),
            &QmlWaveformUntilMark::textSizeChanged,
            pRenderer.get(),
            &allshader::WaveformRenderMark::setUntilMarkTextSize);

    connect(this,
            &QmlWaveformRendererMark::playMarkerColorChanged,
            pRenderer.get(),
            &allshader::WaveformRenderMark::setPlayMarkerForegroundColor);
    connect(this,
            &QmlWaveformRendererMark::playMarkerBackgroundChanged,
            pRenderer.get(),
            &allshader::WaveformRenderMark::setPlayMarkerBackgroundColor);

    // The initialisation is closely inspired from WaveformMarkSet::setup
    int priority = 0;
    for (const auto* pMark : std::as_const(m_marks)) {
        pRenderer->addMark(WaveformMarkPointer(new WaveformMark(
                waveformWidget->getGroup(),
                pMark->control(),
                pMark->visibilityControl(),
                pMark->textColor(),
                pMark->align(),
                pMark->text(),
                pMark->pixmap().toLocalFile(),
                pMark->icon().toLocalFile(),
                pMark->color(),
                priority,
                Cue::kNoHotCue,
                {},
                pMark->endPixmap().toLocalFile(),
                pMark->endIcon().toLocalFile(),
                pMark->disabledOpacity(),
                pMark->enabledOpacity())));
        priority--;
    }
    const auto* pMark = defaultMark();
    if (pMark != nullptr) {
        const QString pixmap = pMark->pixmap().toLocalFile();
        const QString endPixmap = pMark->endPixmap().toLocalFile();
        const QString icon = pMark->icon().toLocalFile();
        const QString endIcon = pMark->endIcon().toLocalFile();
        // FIXME: the following checks should be done on the WaveformMarker
        // setter (depends of #14515)
        if (!pixmap.isEmpty() && !QFileInfo(pixmap).exists()) {
            qmlEngine(this)->throwError(tr("Cannot find the marker pixmap") + " \"" + pixmap + '"');
        }

        if (!endPixmap.isEmpty() && !QFileInfo(endPixmap).exists()) {
            qmlEngine(this)->throwError(tr("Cannot find the marker endPixmap") +
                    " \"" + endPixmap + '"');
        }

        if (!icon.isEmpty() && !QFileInfo(icon).exists()) {
            qmlEngine(this)->throwError(tr("Cannot find the marker icon") + " \"" + icon + '"');
        }

        if (!endIcon.isEmpty() && !QFileInfo(endIcon).exists()) {
            qmlEngine(this)->throwError(tr("Cannot find the marker endIcon") +
                    " \"" + endIcon + '"');
        }
        pRenderer->setDefaultMark(
                waveformWidget->getGroup(),
                WaveformMarkSet::DefaultMarkerStyle{
                        pMark->control(),
                        pMark->visibilityControl(),
                        pMark->textColor(),
                        pMark->align(),
                        pMark->text(),
                        pixmap,
                        endPixmap,
                        icon,
                        endIcon,
                        pMark->color(),
                        pMark->enabledOpacity(),
                        pMark->disabledOpacity(),
                });
    }
    return QmlWaveformRendererFactory::Renderer{pRenderer.get(), std::move(pRenderer)};
}
} // namespace qml
} // namespace mixxx
