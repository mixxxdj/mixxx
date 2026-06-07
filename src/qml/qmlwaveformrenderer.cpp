#include "qml/qmlwaveformrenderer.h"

#include <qapplication.h>
#include <qfileinfo.h>
#include <qglobal.h>
#include <qlist.h>

#include <memory>

#include "moc_qmlwaveformrenderer.cpp"
#include "util/assert.h"
#include "waveform/renderers/allshader/waveformrenderbarcounter.h"
#include "waveform/renderers/allshader/waveformrenderbeat.h"
#include "waveform/renderers/allshader/waveformrendererendoftrack.h"
#include "waveform/renderers/allshader/waveformrendererfiltered.h"
#include "waveform/renderers/allshader/waveformrendererhsv.h"
#include "waveform/renderers/allshader/waveformrendererpreroll.h"
#include "waveform/renderers/allshader/waveformrendererrgb.h"
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"
#include "waveform/renderers/allshader/waveformrenderersimple.h"
// #include "waveform/renderers/allshader/waveformrenderertextured.h"
#include "waveform/renderers/waveformmark.h"
#ifdef __STEM__
#include "waveform/renderers/allshader/waveformrendererstem.h"
#endif
#include "waveform/renderers/allshader/waveformrendermark.h"
#include "waveform/renderers/allshader/waveformrendermarkrange.h"

namespace {
QString waveformMarkerErrorToString(
        const WaveformMark::WaveformMarkConstructionError error,
        const QString& endIcon,
        const QString& pixmap,
        const QString& endPixmap,
        const QString& icon) {
    QString errorMessage = QObject::tr("Invalid marker: ");
    switch (error) {
    case WaveformMark::WaveformMarkConstructionError::EndIconInvalidArgumentCount:
        errorMessage.append(
                QObject::tr("unexpected number of arguments in endIcon: %1")
                        .arg(endIcon));
        break;
    case WaveformMark::WaveformMarkConstructionError::PixmapNotFound:
        errorMessage.append(QObject::tr("path %1 for pixmap cannot be found").arg(pixmap));
        break;
    case WaveformMark::WaveformMarkConstructionError::EndPixmapNotFound:
        errorMessage.append(
                QObject::tr("path %1 for end pixmap cannot be found")
                        .arg(endPixmap));
        break;
    case WaveformMark::WaveformMarkConstructionError::IconNotFound:
        errorMessage.append(QObject::tr("path %1 for icon cannot be found").arg(icon));
        break;
    case WaveformMark::WaveformMarkConstructionError::EndIconNotFound:
        errorMessage.append(QObject::tr("path %1 for end icon cannot be found").arg(endIcon));
        break;
    default:
        DEBUG_ASSERT(!"unreachable");
        errorMessage.append("unknown error");
    }
    return errorMessage;
}
} // namespace

namespace mixxx {
namespace qml {

QmlWaveformRendererMark::QmlWaveformRendererMark()
        : m_playMarkerPosition(0.5),
          m_defaultMark(nullptr),
          m_untilMark(std::make_unique<QmlWaveformUntilMark>()) {
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererEndOfTrack::create(
        WaveformWidgetRenderer* waveformWidget,
        mixxx::qml::WaveformRendererSignalBaseOptions) const {
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
        WaveformWidgetRenderer* waveformWidget,
        mixxx::qml::WaveformRendererSignalBaseOptions options) const {
    auto pRenderer = std::make_unique<allshader::WaveformRendererPreroll>(
            waveformWidget, m_position);
    pRenderer->setColor(m_color);
    connect(this,
            &QmlWaveformRendererPreroll::colorChanged,
            pRenderer.get(),
            &allshader::WaveformRendererPreroll::setColor);
    return QmlWaveformRendererFactory::Renderer{pRenderer.get(), std::move(pRenderer)};
}

void QmlWaveformRendererSignal::setup(
        allshader::WaveformRendererSignalBase* pRenderer) const {
    pRenderer->setAxesColor(m_axesColor);
    pRenderer->setLowColor(m_lowColor);
    pRenderer->setMidColor(m_midColor);
    pRenderer->setHighColor(m_highColor);
    connect(this,
            &QmlWaveformRendererSignal::axesColorChanged,
            pRenderer,
            &allshader::WaveformRendererSignalBase::setAxesColor);
    connect(this,
            &QmlWaveformRendererSignal::lowColorChanged,
            pRenderer,
            &allshader::WaveformRendererSignalBase::setLowColor);
    connect(this,
            &QmlWaveformRendererSignal::midColorChanged,
            pRenderer,
            &allshader::WaveformRendererSignalBase::setMidColor);
    connect(this,
            &QmlWaveformRendererSignal::highColorChanged,
            pRenderer,
            &allshader::WaveformRendererSignalBase::setHighColor);

    pRenderer->setAllChannelVisualGain(m_gainAll);
    pRenderer->setLowVisualGain(m_gainLow);
    pRenderer->setMidVisualGain(m_gainMid);
    pRenderer->setHighVisualGain(m_gainHigh);
    connect(this,
            &QmlWaveformRendererSignal::gainAllChanged,
            pRenderer,
            &allshader::WaveformRendererSignalBase::setAllChannelVisualGain);
    connect(this,
            &QmlWaveformRendererSignal::gainLowChanged,
            pRenderer,
            &allshader::WaveformRendererSignalBase::setLowVisualGain);
    connect(this,
            &QmlWaveformRendererSignal::gainMidChanged,
            pRenderer,
            &allshader::WaveformRendererSignalBase::setMidVisualGain);
    connect(this,
            &QmlWaveformRendererSignal::gainHighChanged,
            pRenderer,
            &allshader::WaveformRendererSignalBase::setHighVisualGain);
    pRenderer->setIgnoreStem(m_ignoreStem);
    connect(this,
            &QmlWaveformRendererSignal::ignoreStemChanged,
            pRenderer,
            &allshader::WaveformRendererSignalBase::setIgnoreStem);
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererRGB::create(
        WaveformWidgetRenderer* waveformWidget,
        mixxx::qml::WaveformRendererSignalBaseOptions options) const {
    std::unique_ptr<rendergraph::BaseNode> pRenderer;

    if (options & allshader::WaveformRendererSignalBase::Option::HighDetail) {
        // FIXME WaveformRendererTextured is currently not supported on SG (#14990)
        // pRenderer.reset(new allshader::WaveformRendererTextured(
        //     waveformWidget, ::WaveformWidgetType::RGB, m_position, options));
        qWarning() << "Waveform high details option is currently not supported "
                      "on scene graph backend. Ignoring";
        options ^= allshader::WaveformRendererSignalBase::Option::HighDetail;
    }
    pRenderer.reset(new allshader::WaveformRendererRGB(
            waveformWidget, m_position, options));
    setup(dynamic_cast<allshader::WaveformRendererSignalBase*>(pRenderer.get()));

    return QmlWaveformRendererFactory::Renderer{
            dynamic_cast<::WaveformRendererAbstract*>(pRenderer.get()),
            std::move(pRenderer)};
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererFiltered::create(
        WaveformWidgetRenderer* waveformWidget,
        mixxx::qml::WaveformRendererSignalBaseOptions options) const {
    std::unique_ptr<rendergraph::BaseNode> pRenderer;

    if (options & allshader::WaveformRendererSignalBase::Option::HighDetail) {
        // FIXME WaveformRendererTextured is currently not supported on SG
        // (#14990) pRenderer.reset(new allshader::WaveformRendererTextured(
        //     waveformWidget, m_stacked ? ::WaveformWidgetType::Stacked :
        //     ::WaveformWidgetType::Filtered, m_position, options));
        qWarning() << "Waveform high details option is currently not supported "
                      "on scene graph backend. Ignoring";
        options ^= allshader::WaveformRendererSignalBase::Option::HighDetail;
    }
    pRenderer.reset(new allshader::WaveformRendererFiltered(
            waveformWidget, m_stacked, options));
    setup(dynamic_cast<allshader::WaveformRendererSignalBase*>(pRenderer.get()));

    return QmlWaveformRendererFactory::Renderer{
            dynamic_cast<::WaveformRendererAbstract*>(pRenderer.get()),
            std::move(pRenderer)};
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererHSV::create(
        WaveformWidgetRenderer* waveformWidget,
        mixxx::qml::WaveformRendererSignalBaseOptions options) const {
    auto pRenderer = std::make_unique<allshader::WaveformRendererHSV>(
            waveformWidget, options);

    pRenderer->setAxesColor(m_axesColor);
    pRenderer->setColor(m_color);
    pRenderer->setIgnoreStem(m_ignoreStem);
    pRenderer->setAllChannelVisualGain(m_gainAll);
    pRenderer->setLowVisualGain(m_gainLow);
    pRenderer->setMidVisualGain(m_gainMid);
    pRenderer->setHighVisualGain(m_gainHigh);
    connect(this,
            &QmlWaveformRendererHSV::gainAllChanged,
            pRenderer.get(),
            &allshader::WaveformRendererSignalBase::setAllChannelVisualGain);
    connect(this,
            &QmlWaveformRendererHSV::gainLowChanged,
            pRenderer.get(),
            &allshader::WaveformRendererSignalBase::setLowVisualGain);
    connect(this,
            &QmlWaveformRendererHSV::gainMidChanged,
            pRenderer.get(),
            &allshader::WaveformRendererSignalBase::setMidVisualGain);
    connect(this,
            &QmlWaveformRendererHSV::gainHighChanged,
            pRenderer.get(),
            &allshader::WaveformRendererSignalBase::setHighVisualGain);
    connect(this,
            &QmlWaveformRendererHSV::axesColorChanged,
            pRenderer.get(),
            &allshader::WaveformRendererSignalBase::setAxesColor);
    connect(this,
            &QmlWaveformRendererHSV::colorChanged,
            pRenderer.get(),
            &allshader::WaveformRendererSignalBase::setColor);
    connect(this,
            &QmlWaveformRendererHSV::ignoreStemChanged,
            pRenderer.get(),
            &allshader::WaveformRendererSignalBase::setIgnoreStem);
    return QmlWaveformRendererFactory::Renderer{pRenderer.get(), std::move(pRenderer)};
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererSimple::create(
        WaveformWidgetRenderer* waveformWidget,
        mixxx::qml::WaveformRendererSignalBaseOptions options) const {
    auto pRenderer = std::make_unique<allshader::WaveformRendererSimple>(
            waveformWidget, options);

    pRenderer->setAxesColor(m_axesColor);
    pRenderer->setColor(m_color);
    pRenderer->setAllChannelVisualGain(m_gain);
    pRenderer->setIgnoreStem(m_ignoreStem);
    connect(this,
            &QmlWaveformRendererSimple::axesColorChanged,
            pRenderer.get(),
            &allshader::WaveformRendererSignalBase::setAxesColor);
    connect(this,
            &QmlWaveformRendererSimple::colorChanged,
            pRenderer.get(),
            &allshader::WaveformRendererSignalBase::setColor);
    connect(this,
            &QmlWaveformRendererSimple::gainChanged,
            pRenderer.get(),
            &allshader::WaveformRendererSignalBase::setAllChannelVisualGain);
    connect(this,
            &QmlWaveformRendererSimple::ignoreStemChanged,
            pRenderer.get(),
            &allshader::WaveformRendererSignalBase::setIgnoreStem);
    return QmlWaveformRendererFactory::Renderer{pRenderer.get(), std::move(pRenderer)};
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererBeat::create(
        WaveformWidgetRenderer* waveformWidget,
        mixxx::qml::WaveformRendererSignalBaseOptions options) const {
    auto pRenderer = std::make_unique<allshader::WaveformRenderBeat>(
            waveformWidget, m_position);
    waveformWidget->setDisplayBeatGridAlpha(m_color.alphaF() * 100);
    pRenderer->setColor(m_color.rgb());
    pRenderer->setDownbeatColor(m_downbeatColor);
    pRenderer->setBeatsPerBar(m_beatsPerBar);
    connect(this,
            &QmlWaveformRendererBeat::colorChanged,
            pRenderer.get(),
            [waveformWidget, pRenderer = pRenderer.get()](const QColor& color) {
                waveformWidget->setDisplayBeatGridAlpha(color.alphaF() * 100);
                pRenderer->setColor(color.rgb());
            });
    connect(this,
            &QmlWaveformRendererBeat::downbeatColorChanged,
            pRenderer.get(),
            [pRenderer = pRenderer.get()](const QColor& color) {
                pRenderer->setDownbeatColor(color);
            });
    connect(this,
            &QmlWaveformRendererBeat::beatsPerBarChanged,
            pRenderer.get(),
            [pRenderer = pRenderer.get()](int beatsPerBar) {
                pRenderer->setBeatsPerBar(beatsPerBar);
            });
    return QmlWaveformRendererFactory::Renderer{pRenderer.get(), std::move(pRenderer)};
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererBarCounter::create(
        WaveformWidgetRenderer* waveformWidget,
        mixxx::qml::WaveformRendererSignalBaseOptions) const {
    auto pRenderer = std::make_unique<allshader::WaveformRenderBarCounter>(
            waveformWidget, m_position);
    pRenderer->setColor(m_color);
    pRenderer->setBeatsPerBar(m_beatsPerBar);
    connect(this,
            &QmlWaveformRendererBarCounter::colorChanged,
            pRenderer.get(),
            [pRenderer = pRenderer.get()](const QColor& color) {
                pRenderer->setColor(color);
            });
    connect(this,
            &QmlWaveformRendererBarCounter::beatsPerBarChanged,
            pRenderer.get(),
            [pRenderer = pRenderer.get()](int beatsPerBar) {
                pRenderer->setBeatsPerBar(beatsPerBar);
            });
    return QmlWaveformRendererFactory::Renderer{pRenderer.get(), std::move(pRenderer)};
}

QmlWaveformRendererFactory::Renderer QmlWaveformRendererMarkRange::create(
        WaveformWidgetRenderer* waveformWidget,
        mixxx::qml::WaveformRendererSignalBaseOptions options) const {
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
        WaveformWidgetRenderer* waveformWidget,
        mixxx::qml::WaveformRendererSignalBaseOptions options) const {
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
        WaveformWidgetRenderer* waveformWidget,
        mixxx::qml::WaveformRendererSignalBaseOptions options) const {
    VERIFY_OR_DEBUG_ASSERT(!!m_untilMark) {
        return QmlWaveformRendererFactory::Renderer{};
    }
    auto pRenderer = std::make_unique<allshader::WaveformRenderMark>(waveformWidget,
            ::WaveformRendererAbstract::Play);

    pRenderer->setPlayMarkerForegroundColor(m_playMarkerColor);
    pRenderer->setPlayMarkerBackgroundColor(m_playMarkerBackground);
    waveformWidget->setPlayMarkerPosition(m_playMarkerPosition);

    pRenderer->setUntilMarkShowBeats(m_untilMark->showTime());
    pRenderer->setUntilMarkShowTime(m_untilMark->showBeats());
    pRenderer->setUntilMarkAlign(m_untilMark->align());
    pRenderer->setUntilMarkTextSize(m_untilMark->textSize());
    pRenderer->setUntilMarkTextHeightLimit(m_untilMark->textHeightLimit());
    pRenderer->setDefaultNextMarkPosition(m_untilMark->defaultNextMarkPosition());

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
    connect(m_untilMark.get(),
            &QmlWaveformUntilMark::defaultNextMarkPositionChanged,
            pRenderer.get(),
            &allshader::WaveformRenderMark::setDefaultNextMarkPosition);

    connect(this,
            &QmlWaveformRendererMark::playMarkerColorChanged,
            pRenderer.get(),
            &allshader::WaveformRenderMark::setPlayMarkerForegroundColor);
    connect(this,
            &QmlWaveformRendererMark::playMarkerBackgroundChanged,
            pRenderer.get(),
            &allshader::WaveformRenderMark::setPlayMarkerBackgroundColor);
    connect(this,
            &QmlWaveformRendererMark::playMarkerPositionChanged,
            pRenderer.get(),
            [waveformWidget](double value) {
                waveformWidget->setPlayMarkerPosition(value);
            });

    // The initialisation is closely inspired from WaveformMarkSet::setup
    int priority = 0;
    for (const auto* pMark : std::as_const(m_marks)) {
        const QString pixmap = pMark->pixmap().toLocalFile();
        const QString icon = pMark->icon().toLocalFile();
        const QString endPixmap = pMark->endPixmap().toLocalFile();
        const QString endIcon = pMark->endIcon().toLocalFile();
        auto maybeMarker = WaveformMark::create(
                waveformWidget->getGroup(),
                pMark->control(),
                pMark->visibilityControl(),
                pMark->textColor(),
                pMark->align(),
                pMark->text(),
                pixmap,
                icon,
                pMark->color(),
                priority,
                Cue::kNoHotCue,
                {},
                endPixmap,
                endIcon,
                pMark->disabledOpacity(),
                pMark->enabledOpacity());

        if (std::holds_alternative<WaveformMark::WaveformMarkConstructionError>(maybeMarker)) {
            qmlEngine(this)->throwError(waveformMarkerErrorToString(
                    std::get<WaveformMark::WaveformMarkConstructionError>(
                            maybeMarker),
                    endIcon,
                    pixmap,
                    endPixmap,
                    icon));
            continue;
        }
        auto pMarker = std::get<WaveformMarkPointer>(maybeMarker);
        pRenderer->addMark(pMarker);
        priority--;
    }
    const auto* pMark = defaultMark();
    if (pMark != nullptr) {
        const QString pixmap = pMark->pixmap().toLocalFile();
        const QString endPixmap = pMark->endPixmap().toLocalFile();
        const QString icon = pMark->icon().toLocalFile();
        const QString endIcon = pMark->endIcon().toLocalFile();
        auto error = pRenderer->setDefaultMark(
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
        if (error.has_value()) {
            qmlEngine(this)->throwError(waveformMarkerErrorToString(
                    error.value(), endIcon, pixmap, endPixmap, icon));
        }
    }
    return QmlWaveformRendererFactory::Renderer{pRenderer.get(), std::move(pRenderer)};
}
} // namespace qml
} // namespace mixxx
