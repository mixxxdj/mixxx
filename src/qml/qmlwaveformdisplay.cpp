#include "qml/qmlwaveformdisplay.h"

#include <QQuickWindow>
#include <QSGFlatColorMaterial>
#include <QSGSimpleRectNode>
#include <QSGVertexColorMaterial>
#include <Qt>
#include <QtQuick/QSGGeometryNode>
#include <QtQuick/QSGMaterial>
#include <QtQuick/QSGRectangleNode>
#include <QtQuick/QSGTexture>
#include <QtQuick/QSGTextureProvider>
#include <algorithm>
#include <cmath>

#include "library/library.h"
#include "mixer/basetrackplayer.h"
#include "moc_qmlwaveformdisplay.cpp"
#include "qml/qmlplayerproxy.h"
#include "qmllibraryproxy.h"
#include "rendergraph/context.h"
#include "rendergraph/node.h"
#include "util/assert.h"
#include "waveform/visualplayposition.h"

using namespace allshader;

namespace {
constexpr int kDefaultSyncInternalMs = 100;
} // namespace

namespace mixxx {
namespace qml {

QmlWaveformDisplay::QmlWaveformDisplay(QQuickItem* parent)
        : QQuickItem(parent),
          WaveformWidgetRenderer(),
          m_syncInterval(kDefaultSyncInternalMs),
          m_pPlayer(nullptr),
          m_pTrack(nullptr),
          m_visualPlayPosition(QSharedPointer<VisualPlayPosition>::create()) {
    m_visualPlayPosition->set(0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            SlipModeState::Disabled,
            false,
            false,
            false,
            0.0,
            0.0,
            0.0,
            0.0);
    setFlag(QQuickItem::ItemHasContents, true);

    connect(this,
            &QmlWaveformDisplay::windowChanged,
            this,
            &QmlWaveformDisplay::slotWindowChanged,
            Qt::DirectConnection);
    slotWindowChanged(window());
}

QmlWaveformDisplay::~QmlWaveformDisplay() {
    // The stack contains references to Renderer that are owned and cleared by a BaseNode
    m_rendererStack.clear();
}

void QmlWaveformDisplay::componentComplete() {
    if (!getGroup().isEmpty()) {
        qDebug() << "QmlWaveformDisplay ready for group" << getGroup() << "with"
                 << m_waveformRenderers.count() << "renderer(s)";
    } else if (m_pTrack && m_pTrack->internal()) {
        qDebug() << "QmlWaveformDisplay ready for track" << m_pTrack->internal().get() << "with"
                 << m_waveformRenderers.count() << "renderer(s)";
    } else {
        qWarning() << "QmlWaveformDisplay initialised with neither a track nor a group!";
    }
    QQuickItem::componentComplete();
}

double QmlWaveformDisplay::getPosition() const {
    return m_visualPlayPosition->getEnginePlayPos();
}

void QmlWaveformDisplay::setPosition(double value) {
    if (!m_pTrack || !m_pTrack->internal()) {
        return;
    }
    m_visualPlayPosition->set(std::clamp(value, 0.0, 1.0),
            1.0,
            static_cast<int>(1024) /
                    (m_pTrack->internal()->getBitrate() *
                            m_pTrack->internal()->getDuration()),
            0.0,
            0.0,
            SlipModeState::Disabled,
            false,
            false,
            false,
            0,
            0,
            m_pTrack->internal()->getDuration(),
            1024 / mixxx::kEngineChannelOutputCount /
                    m_pTrack->internal()->getSampleRate() * 1000000.0);
    update();
}

void QmlWaveformDisplay::setStaticTrack(QmlTrackProxy* track) {
    m_pTrack = track;
    if (!m_pTrack) {
        if (!m_pPlayer) {
            setCurrentTrack(nullptr);
        }
        return;
    }
    if (!track->internal()) {
        return;
    }
    if (!track->internal()->getWaveform()) {
        emit QmlLibraryProxy::get() -> analyzeTracks({track->internal()->getId()});
    }
    setVisualPlayPosition(m_visualPlayPosition);
    m_visualPlayPosition->set(0.0,
            1.0,
            static_cast<int>(1024) /
                    (m_pTrack->internal()->getBitrate() *
                            m_pTrack->internal()->getDuration()),
            0.0,
            0.0,
            SlipModeState::Disabled,
            false,
            false,
            false,
            0,
            0,
            m_pTrack->internal()->getDuration(),
            1024 / mixxx::kEngineChannelOutputCount /
                    m_pTrack->internal()->getSampleRate() * 1000000.0);
    setCurrentTrack(track->internal());
    emit trackChanged(track);
    emit groupChanged("");
    update();
}

void QmlWaveformDisplay::slotWindowChanged(QQuickWindow* window) {
    m_rendererStack.clear();

    m_dirtyFlag.setFlag(DirtyFlag::Window, true);
    if (window) {
        connect(window, &QQuickWindow::afterFrameEnd, this, &QmlWaveformDisplay::slotFrameSwapped);
    }
    m_timer.restart();
}

void QmlWaveformDisplay::setOptions(mixxx::qml::WaveformRendererSignalBaseOptions options) {
    m_options = options;
    emit optionsChanged(options);
    m_rendererStack.clear();

    m_dirtyFlag.setFlag(DirtyFlag::Window, true);
}

std::chrono::microseconds QmlWaveformDisplay::fromTimerToNextSync(const PerformanceTimer& timer) {
    // TODO @m0dB probably better to use a singleton instead of deriving QmlWaveformDisplay from
    // ISyncTimeProvider and have each keep track of this.
    if (m_pTrack) {
        return m_syncInterval;
    }
    return m_syncInterval + std::chrono::microseconds(m_timer.difference(timer).toIntegerMicros());
}

void QmlWaveformDisplay::slotFrameSwapped() {
    m_timer.restart();

    // continuous redraw
    update();
}

void QmlWaveformDisplay::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) {
    m_dirtyFlag.setFlag(DirtyFlag::Geometry, true);
    update();
    QQuickItem::geometryChange(newGeometry, oldGeometry);
}

QSGNode* QmlWaveformDisplay::updatePaintNode(QSGNode* node, UpdatePaintNodeData*) {
    if (m_dirtyFlag.testFlag(DirtyFlag::Window)) {
        delete node;
        node = nullptr;
        m_dirtyFlag.setFlag(DirtyFlag::Window, false);
    }

    DEBUG_ASSERT(!node || node->childCount() == 1);

    auto* pClipNode = dynamic_cast<QSGClipNode*>(node);
    auto* pBgNode = pClipNode ? dynamic_cast<QSGSimpleRectNode*>(pClipNode->lastChild()) : nullptr;

    if (!pBgNode) {
        if (pClipNode) {
            delete pClipNode;
        }
        pClipNode = new QSGClipNode();
        pBgNode = new QSGSimpleRectNode();
        m_dirtyFlag.setFlag(DirtyFlag::Background, true);
        m_dirtyFlag.setFlag(DirtyFlag::Geometry, true);

        pClipNode->setIsRectangular(true);
        pClipNode->setGeometry(pBgNode->geometry());

        pClipNode->setClipRect(boundingRect());
        pBgNode->setRect(boundingRect());

        if (getContext()) {
            delete getContext();
        }
        setContext(new rendergraph::Context(window()));
        rendergraph::Node* pTopNode = new rendergraph::Node;

        m_rendererStack.clear();
        for (auto* pQmlRenderer : std::as_const(m_waveformRenderers)) {
            if (!pQmlRenderer->isSupported()) {
                qDebug() << "Ignoring the unsupported" << pQmlRenderer << "renderer";
            }
            auto renderer = pQmlRenderer->create(this, m_options);
            // FIXME Some renderer will return nullptr till
            // WaveformRendererTextured is supported (#14990)
            // It is also expected for the stem renderer to return a null value in
            // case STEM are not enabled.
            if (!renderer.renderer) {
                continue;
            }
            addRenderer(renderer.renderer);
            pTopNode->appendChildNode(std::move(renderer.node));
        }

        pBgNode->appendChildNode(pTopNode);
        pClipNode->appendChildNode(pBgNode);
        init();
    }

    if (m_dirtyFlag.testFlag(DirtyFlag::Background)) {
        m_dirtyFlag.setFlag(DirtyFlag::Background, false);
        pBgNode->setColor(m_backgroundColor);
    }

    if (m_dirtyFlag.testFlag(DirtyFlag::Geometry)) {
        m_dirtyFlag.setFlag(DirtyFlag::Geometry, false);
        pBgNode->setRect(boundingRect());
        pClipNode->setClipRect(boundingRect());
        resizeRenderer(boundingRect().width(),
                boundingRect().height(),
                window()->devicePixelRatio());
    }

    onPreRender(this);

    for (auto* pRenderer : std::as_const(m_rendererStack)) {
        pRenderer->update();
    }

    pBgNode->markDirty(QSGNode::DirtyForceUpdate);
    return pClipNode;
}

QmlPlayerProxy* QmlWaveformDisplay::getPlayer() const {
    return m_pPlayer;
}

void QmlWaveformDisplay::setPlayer(QmlPlayerProxy* pPlayer) {
    if (m_pPlayer == pPlayer) {
        return;
    }

    if (m_pPlayer != nullptr) {
        m_pPlayer->internalTrackPlayer()->disconnect(this);
    }

    m_pPlayer = pPlayer;

    if (m_pPlayer != nullptr) {
        setGroup(m_pPlayer->internalTrackPlayer()->getGroup());
        setCurrentTrack(m_pPlayer->internalTrackPlayer()->getLoadedTrack());
        connect(m_pPlayer->internalTrackPlayer(),
                &BaseTrackPlayer::newTrackLoaded,
                this,
                &QmlWaveformDisplay::slotTrackLoaded);
        connect(m_pPlayer->internalTrackPlayer(),
                &BaseTrackPlayer::loadingTrack,
                this,
                &QmlWaveformDisplay::slotTrackLoading);
        connect(m_pPlayer->internalTrackPlayer(),
                &BaseTrackPlayer::playerEmpty,
                this,
                &QmlWaveformDisplay::slotTrackUnloaded);
    }

    emit playerChanged();
    update();
}

void QmlWaveformDisplay::setGroup(const QString& group) {
    if (getGroup() == group) {
        return;
    }

    WaveformWidgetRenderer::setGroup(group);
    emit groupChanged(group);
}

void QmlWaveformDisplay::slotTrackLoaded(TrackPointer pTrack) {
    // TODO: Investigate if it's a bug that this debug assertion fails when
    // passing tracks on the command line
    // DEBUG_ASSERT(m_pCurrentTrack == pTrack);
    setCurrentTrack(pTrack);
}

void QmlWaveformDisplay::slotTrackLoading(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pOldTrack); // only used in DEBUG_ASSERT
    DEBUG_ASSERT(getTrackInfo() == pOldTrack);
    setCurrentTrack(pNewTrack);
}

void QmlWaveformDisplay::slotTrackUnloaded() {
    setCurrentTrack(nullptr);
}

void QmlWaveformDisplay::setCurrentTrack(TrackPointer pTrack) {
    auto pCurrentTrack = getTrackInfo();
    // TODO: Check if this is actually possible
    if (pCurrentTrack == pTrack) {
        return;
    }

    if (pCurrentTrack != nullptr) {
        disconnect(pCurrentTrack.get(), nullptr, this, nullptr);
    }

    setTrack(pTrack);
    if (pTrack != nullptr) {
        connect(pTrack.get(),
                &Track::waveformSummaryUpdated,
                this,
                &QmlWaveformDisplay::slotWaveformUpdated);
    }
    slotWaveformUpdated();
}

void QmlWaveformDisplay::slotWaveformUpdated() {
    update();
}

QQmlListProperty<QmlWaveformRendererFactory> QmlWaveformDisplay::renderers() {
    return {this,
            nullptr,
            &QmlWaveformDisplay::renderers_append,
            &QmlWaveformDisplay::renderers_count,
            &QmlWaveformDisplay::renderers_at,
            &QmlWaveformDisplay::renderers_clear};
}

// Static
void QmlWaveformDisplay::renderers_append(
        QQmlListProperty<QmlWaveformRendererFactory>* pList,
        QmlWaveformRendererFactory* value) {
    QmlWaveformDisplay* pWaveform = static_cast<QmlWaveformDisplay*>(pList->object);
    VERIFY_OR_DEBUG_ASSERT(pWaveform) {
        return;
    }
    pWaveform->m_dirtyFlag.setFlag(DirtyFlag::Window, true);
    pWaveform->m_waveformRenderers.append(value);
}

// Static
qsizetype QmlWaveformDisplay::renderers_count(QQmlListProperty<QmlWaveformRendererFactory>* pList) {
    QmlWaveformDisplay* pWaveform = static_cast<QmlWaveformDisplay*>(pList->object);
    VERIFY_OR_DEBUG_ASSERT(pWaveform) {
        return 0;
    }
    pWaveform->m_dirtyFlag.setFlag(DirtyFlag::Window, true);
    return pWaveform->m_waveformRenderers.count();
}

// Static
QmlWaveformRendererFactory* QmlWaveformDisplay::renderers_at(
        QQmlListProperty<QmlWaveformRendererFactory>* pList, qsizetype index) {
    VERIFY_OR_DEBUG_ASSERT(pList && pList->object) {
        return nullptr;
    }
    QmlWaveformDisplay* pWaveform = static_cast<QmlWaveformDisplay*>(pList->object);
    VERIFY_OR_DEBUG_ASSERT(pWaveform) {
        return nullptr;
    }
    pWaveform->m_dirtyFlag.setFlag(DirtyFlag::Window, true);
    return pWaveform->m_waveformRenderers.at(index);
}

// Static
void QmlWaveformDisplay::renderers_clear(QQmlListProperty<QmlWaveformRendererFactory>* pList) {
    QmlWaveformDisplay* pWaveform = static_cast<QmlWaveformDisplay*>(pList->object);
    VERIFY_OR_DEBUG_ASSERT(pWaveform) {
        return;
    }
    pWaveform->m_dirtyFlag.setFlag(DirtyFlag::Window, true);
    return pWaveform->m_waveformRenderers.clear();
}

} // namespace qml
} // namespace mixxx
