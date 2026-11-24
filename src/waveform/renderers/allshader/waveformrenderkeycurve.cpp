
#include "waveform/renderers/allshader/waveformrenderkeycurve.h"

#include <QImage>
#include <QPainter>
#include <QSizeF>
#include <QStringView>
#include <cmath>

#include "library/library_prefs.h"
#include "moc_waveformrenderkeycurve.cpp"
#include "proto/keys.pb.h"
#include "rendergraph/context.h"
#include "rendergraph/geometry.h"
#include "rendergraph/geometrynode.h"
#include "rendergraph/material/texturematerial.h"
#include "rendergraph/texture.h"
#include "rendergraph/vertexupdaters/texturedvertexupdater.h"
#include "track/keyutils.h"
#include "track/track.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

using mixxx::track::io::key::ChromaticKey;

using namespace rendergraph;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class WaveformKeyCurveNode : public rendergraph::GeometryNode {
  public:
    WaveformKeyCurveNode(rendergraph::Context* pContext,
            const QImage& image) {
        initForRectangles<TextureMaterial>(1);
        updateTexture(pContext, image);
    }

    void updateTexture(rendergraph::Context* pContext, const QImage& image) {
        dynamic_cast<TextureMaterial&>(material())
                .setTexture(std::make_unique<Texture>(pContext, image));
        m_textureWidth = static_cast<float>(image.width());
        m_textureHeight = static_cast<float>(image.height());
    }

    void updatePosition(float x, float y, float devicePixelRatio) {
        TexturedVertexUpdater vertexUpdater{
                geometry().vertexDataAs<Geometry::TexturedPoint2D>()};
        vertexUpdater.addRectangle({x, y},
                {x + m_textureWidth / devicePixelRatio,
                        y + m_textureHeight / devicePixelRatio},
                {0.f, 0.f},
                {1.f, 1.f});
        markDirtyGeometry();
        markDirtyMaterial();
    }

    float textureWidth() const {
        return m_textureWidth;
    }
    float textureHeight() const {
        return m_textureHeight;
    }

  private:
    float m_textureWidth{};
    float m_textureHeight{};
};

namespace allshader {

constexpr double WHEEL_START_ANGLE = 45.0;
constexpr double SLICE_ANGLE = 30.0;
constexpr int NUM_SLICES = 12;
constexpr bool showDebugAllshaderWaveformRenderKeyCurve = false;
static constexpr int kReloadCheckIntervalMs = 2000;

WaveformRenderKeyCurve::WaveformRenderKeyCurve(
        WaveformWidgetRenderer* waveformWidget,
        ::WaveformRendererAbstract::PositionSource type)
        : WaveformRenderKeyCurveBase(waveformWidget, false),
          m_segments(),
          m_lancelotLayout(),
          m_style(),
          m_animationTimer(),
          m_reloadTimer(),
          m_lastTrackId(),
          m_currentKey(),
          m_currentLancelot(),
          m_baseLancelot(),
          m_transposedLancelot(),
          m_transposedMusicalKey(),
          m_transposedWheelKey(),
          m_currentWheelKey(),
          m_pKeyCurveNode(nullptr),
          m_pKeyCurveNodesParent(nullptr),
          m_pPlayPositionCO(nullptr),
          m_pRateRatioCO(nullptr),
          m_pKeyControlCO(nullptr),
          m_pKeylockCO(nullptr),
          m_pKeyNotationCO(nullptr),
          m_playPosition(0.0),
          m_currentRateRatio(1.0),
          m_currentKeyShift(0.0),
          m_currentPlayPosition(0.0),
          m_trackLengthSeconds(0.0),
          m_wheelSize(120),
          m_wheelMargin(10),
          m_currentTotalOffset(0),
          m_currentKeyId(0),
          m_visible(true),
          m_keylockEnabled(false),
          m_isSlipRenderer(type == ::WaveformRendererAbstract::Slip),
          m_textureReady(false),
          m_previousRendererSize(0, 0) {
    m_animationTimer.start();
    m_reloadTimer.start();
    m_style = KeyCurveStyle();

    auto pNode = std::make_unique<rendergraph::Node>();
    m_pKeyCurveNodesParent = pNode.get();
    appendChildNode(std::move(pNode));

    m_pKeyNotationCO = std::make_unique<ControlProxy>(
            mixxx::library::prefs::kKeyNotationConfigKey, this);

    if (!m_waveformRenderer->getGroup().isEmpty()) {
        m_pPlayPositionCO = std::make_unique<ControlProxy>(
                m_waveformRenderer->getGroup(), "playposition");
        m_playPosition = m_pPlayPositionCO->get();
        m_pKeylockCO = std::make_unique<ControlProxy>(
                m_waveformRenderer->getGroup(), "keylock");
        m_pRateRatioCO = std::make_unique<ControlProxy>(
                m_waveformRenderer->getGroup(), "rate_ratio");
        m_pKeyControlCO = std::make_unique<ControlProxy>(
                m_waveformRenderer->getGroup(), "pitch");
    }

    PollingControlProxy proxyShowLancelotWheel(QStringLiteral("[Waveform]"),
            QStringLiteral("show_lancelot_wheel"),
            ControlFlag::AllowMissingOrInvalid);
    m_showLancelotWheel = proxyShowLancelotWheel.get() > 0 ? true : false;

    PollingControlProxy proxyShowKeyMarkers(QStringLiteral("[Waveform]"),
            QStringLiteral("show_key_markers"),
            ControlFlag::AllowMissingOrInvalid);
    m_showKeyMarkers = proxyShowKeyMarkers.get() > 0 ? true : false;

    PollingControlProxy proxyShowKeyLabels(QStringLiteral("[Waveform]"),
            QStringLiteral("show_key_markers"),
            ControlFlag::AllowMissingOrInvalid);
    m_showKeyLabels = proxyShowKeyLabels.get() > 0 ? true : false;

    initLancelotLayout();
}

void WaveformRenderKeyCurve::initLancelotLayout() {
    m_lancelotLayout.clear();

    QStringList majorLancelots = {"1B",
            "2B",
            "3B",
            "4B",
            "5B",
            "6B",
            "7B",
            "8B",
            "9B",
            "10B",
            "11B",
            "12B"};
    QStringList minorLancelots = {"1A",
            "2A",
            "3A",
            "4A",
            "5A",
            "6A",
            "7A",
            "8A",
            "9A",
            "10A",
            "11A",
            "12A"};

    for (int i = 0; i < NUM_SLICES; ++i) {
        LancelotKey key;
        key.lancelot = majorLancelots[i];
        double angle = WHEEL_START_ANGLE - (i * SLICE_ANGLE);
        while (angle < 0.0) {
            angle += 360.0;
        }
        while (angle >= 360.0) {
            angle -= 360.0;
        }
        key.angle = angle;
        key.isMinor = false;
        m_lancelotLayout.append(key);
    }

    for (int i = 0; i < NUM_SLICES; ++i) {
        LancelotKey key;
        key.lancelot = minorLancelots[i];
        double angle = WHEEL_START_ANGLE - (i * SLICE_ANGLE);
        while (angle < 0.0) {
            angle += 360.0;
        }
        while (angle >= 360.0) {
            angle -= 360.0;
        }
        key.angle = angle;
        key.isMinor = true;
        m_lancelotLayout.append(key);
    }
}

void WaveformRenderKeyCurve::draw(QPainter* painter, QPaintEvent* event) {
    Q_UNUSED(painter);
    Q_UNUSED(event);
    DEBUG_ASSERT(false);
}

void WaveformRenderKeyCurve::setup(const QDomNode& node, const SkinContext& skinContext) {
    Q_UNUSED(node);
    Q_UNUSED(skinContext);
    WaveformRenderKeyCurveBase::setup(node, skinContext);
}

bool WaveformRenderKeyCurve::init() {
    if (showDebugAllshaderWaveformRenderKeyCurve) {
        qDebug() << "[WaveformRenderKeyCurve - Allshader] init called";
    }
    return true;
}

bool WaveformRenderKeyCurve::isSubtreeBlocked() const {
    return m_isSlipRenderer && !m_waveformRenderer->isSlipActive();
}

QString WaveformRenderKeyCurve::keyIdToLancelot(int keyId) const {
    // keyId: 1-12 = Major, 13-24 = Minor
    bool isMinor = (keyId >= 13);
    int rootNote = (keyId - 1) % 12; // 0-11

    int lancelotNumber;
    if (m_style.wheelType == KeyCurveStyle::WHEEL_MIXXX) {
        // Mixxx wheel (OpenKey) mapping
        // OpenKey numbers: 1-12, starting at 1B = C major
        static const int rootToOpenKey[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        lancelotNumber = rootToOpenKey[rootNote];
    } else {
        // Standard Lancelot wheel mapping
        // Major keys mapping (root to Lancelot number)
        static const int rootToLancelotMajor[12] = {8, 3, 10, 5, 12, 7, 2, 9, 4, 11, 6, 1};
        // Minor keys mapping (root to Lancelot number)
        static const int rootToLancelotMinor[12] = {5, 12, 7, 2, 9, 4, 11, 6, 1, 8, 3, 10};

        if (isMinor) {
            lancelotNumber = rootToLancelotMinor[rootNote];
        } else {
            lancelotNumber = rootToLancelotMajor[rootNote];
        }
    }

    return QString::number(lancelotNumber) + (isMinor ? "A" : "B");
}

void WaveformRenderKeyCurve::updateCurrentKey() {
    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack) {
        if (showDebugAllshaderWaveformRenderKeyCurve) {
            qDebug() << "[WaveformRenderKeyCurve - Allshader] No track info";
        }
        return;
    }

    // Load key curve if not loaded yet (first time only)
    if (!m_segmentsLoaded) {
        if (showDebugAllshaderWaveformRenderKeyCurve) {
            qDebug() << "[WaveformRenderKeyCurve - Allshader] Loading key curve from database...";
        }

        // Load from database via Track object
        QList<KeySegmentsPointer> segments = pTrack->getKeySegments();

        if (!segments.isEmpty()) {
            for (const auto& pSegment : std::as_const(segments)) {
                KeySegment seg;
                seg.startTime = pSegment->getStartTime();
                seg.endTime = pSegment->getRangeEnd();
                seg.duration = pSegment->getDuration();
                seg.keyId = pSegment->getKeyId();
                seg.key = pSegment->getKeyText();
                seg.type = pSegment->getType();
                seg.confidence = pSegment->getConfidence();
                m_segments.append(seg);
            }

            if (showDebugAllshaderWaveformRenderKeyCurve) {
                qDebug() << "[WaveformRenderKeyCurve - Allshader] Loaded"
                         << m_segments.size() << "key segments from database for track:"
                         << pTrack->getTitle()
                         << "ID:" << pTrack->getId().toString();
                if (!m_segments.isEmpty()) {
                    qDebug() << "[WaveformRenderKeyCurve - Allshader] First segment - keyId:"
                             << m_segments[0].keyId << "keyText:" << m_segments[0].key;
                }
            }
        } else {
            if (showDebugAllshaderWaveformRenderKeyCurve) {
                qDebug() << "[WaveformRenderKeyCurve - Allshader] No key segments "
                            "in database for track:"
                         << pTrack->getTitle()
                         << "ID:" << pTrack->getId().toString();
            }
        }
        m_segmentsLoaded = true;
    }

    // If no segments present, try to reload (may be analyzing)
    // This will run every time updateCurrentKey is called until segments are found
    if (m_segments.isEmpty()) {
        // Try to reload from database - maybe analysis just finished
        QList<KeySegmentsPointer> segments = pTrack->getKeySegments();

        if (!segments.isEmpty()) {
            if (showDebugAllshaderWaveformRenderKeyCurve) {
                qDebug() << "[WaveformRenderKeyCurve - Allshader] Reloaded"
                         << segments.size() << "key segments after analysis for track:"
                         << pTrack->getTitle()
                         << "ID:" << pTrack->getId().toString();
            }

            for (const auto& pSegment : std::as_const(segments)) {
                KeySegment seg;
                seg.startTime = pSegment->getStartTime();
                seg.endTime = pSegment->getRangeEnd();
                seg.duration = pSegment->getDuration();
                seg.keyId = pSegment->getKeyId();
                seg.key = pSegment->getKeyText();
                seg.type = pSegment->getType();
                seg.confidence = pSegment->getConfidence();
                m_segments.append(seg);
            }

            // Force complete recreation of the wheel
            if (m_pKeyCurveNode) {
                m_pKeyCurveNodesParent->removeChildNode(m_pKeyCurveNode);
                m_pKeyCurveNode = nullptr;
            }
            m_pendingWheelImage = QImage();

            if (!m_segments.isEmpty()) {
                int notationValue = static_cast<int>(m_pKeyNotationCO->get());
                KeyUtils::KeyNotation notation =
                        KeyUtils::keyNotationFromNumericValue(notationValue);
                m_currentWheelKey = KeyUtils::keyToString(
                        static_cast<ChromaticKey>(m_segments[0].keyId),
                        notation);
                m_baseLancelot = keyIdToLancelot(m_segments[0].keyId);
                updateTransposedKey();

                // Force an immediate update to create the wheel node
                update();
            }
        }
        // If still no segments, just return and try again next time
        if (m_segments.isEmpty()) {
            return;
        }
    }

    // Update track duration
    if (m_trackLengthSeconds == 0.0) {
        m_trackLengthSeconds = pTrack->getDuration();
        if (showDebugAllshaderWaveformRenderKeyCurve) {
            qDebug() << "[WaveformRenderKeyCurve - Allshader] Track duration:"
                     << m_trackLengthSeconds;
        }
    }

    // Update play position
    if (m_pPlayPositionCO) {
        m_playPosition = m_pPlayPositionCO->get();
    }

    // Find current key based on play position
    double positionSeconds = m_playPosition * m_trackLengthSeconds;

    // Get current user notation preference
    int notationValue = static_cast<int>(m_pKeyNotationCO->get());
    KeyUtils::KeyNotation notation = KeyUtils::keyNotationFromNumericValue(notationValue);

    for (const auto& seg : std::as_const(m_segments)) {
        if (positionSeconds >= seg.startTime && positionSeconds <= seg.endTime) {
            QString displayKey = KeyUtils::keyToString(
                    static_cast<ChromaticKey>(seg.keyId), notation);
            if (m_currentKey != displayKey) {
                if (showDebugAllshaderWaveformRenderKeyCurve) {
                    qDebug() << "[WaveformRenderKeyCurve - Allshader] Key changed from"
                             << m_currentKey << "to" << displayKey << "at"
                             << positionSeconds << "s";
                }
                m_currentKey = displayKey;
                m_currentKeyId = seg.keyId;
                m_currentLancelot = keyIdToLancelot(seg.keyId);
                m_currentWheelKey = displayKey;
                m_baseLancelot = keyIdToLancelot(seg.keyId);
                updateTransposedKey();
            }
            break;
        }
    }

    // set initial values if this is the first time
    if (!m_segments.isEmpty() && m_currentWheelKey.isEmpty()) {
        QString displayKey = KeyUtils::keyToString(
                static_cast<ChromaticKey>(m_segments[0].keyId), notation);
        m_currentKey = displayKey;
        m_currentKeyId = m_segments[0].keyId;
        m_currentWheelKey = displayKey;
        m_baseLancelot = keyIdToLancelot(m_segments[0].keyId);
        updateTransposedKey();
    }
}

void WaveformRenderKeyCurve::updateTransposedKey() {
    if (m_baseLancelot.isEmpty() || m_currentWheelKey.isEmpty()) {
        return;
    }
    if (m_pKeylockCO) {
        m_keylockEnabled = (m_pKeylockCO->get() > 0.5);
    }

    double pitchSemitones = m_keylockEnabled ? 0.0 : 12.0 * log2(m_currentRateRatio);
    int totalSemitones = static_cast<int>(std::round(pitchSemitones + m_currentKeyShift));
    m_currentTotalOffset = totalSemitones;

    calculateTransposedValues(totalSemitones);

    if (showDebugAllshaderWaveformRenderKeyCurve) {
        qDebug() << "[WaveformRenderKeyCurve - Allshader] Keylock:" << m_keylockEnabled
                 << "Semitones:" << totalSemitones
                 << "Original:" << m_currentWheelKey << "(" << m_baseLancelot << ")"
                 << "-> Flashing:" << m_transposedLancelot
                 << "Display:" << m_transposedMusicalKey;
    }
}

void WaveformRenderKeyCurve::getVisibleRange(
        double& startSample, double& endSample, double& width, double& height) {
    width = static_cast<float>(m_waveformRenderer->getWidth());
    height = static_cast<float>(m_waveformRenderer->getBreadth());

    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (pTrack) {
        double trackSamples = m_waveformRenderer->getTrackSamples();
        double firstDisplayedPosition = m_waveformRenderer->getFirstDisplayedPosition();
        double lastDisplayedPosition = m_waveformRenderer->getLastDisplayedPosition();

        startSample = firstDisplayedPosition * trackSamples;
        endSample = lastDisplayedPosition * trackSamples;
    }
}

QImage WaveformRenderKeyCurve::createFullImage() {
    float width = static_cast<float>(m_waveformRenderer->getWidth());
    float height = static_cast<float>(m_waveformRenderer->getBreadth());

    QImage image(static_cast<int>(width),
            static_cast<int>(height),
            QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    drawKeyMarkers(painter, width, height);
    drawKeyLabels(painter, width, height);
    drawLancelotWheelComponents(painter, width, height);

    painter.end();

    return image;
}

void WaveformRenderKeyCurve::drawKeyMarkers(QPainter& painter, float width, float height) {
    if (!m_showKeyMarkers) {
        return;
    }
    if (m_segments.isEmpty()) {
        return;
    }

    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack) {
        return;
    }

    double trackLengthSeconds = pTrack->getDuration();
    double trackSamples = m_waveformRenderer->getTrackSamples();
    double startSample, endSample, dummy;
    getVisibleRange(startSample, endSample, dummy, dummy);

    painter.setPen(QPen(m_style.markerColor, m_style.markerWidth, m_style.markerLineStyle));

    for (const auto& seg : std::as_const(m_segments)) {
        double startPos = seg.startTime * (trackSamples / trackLengthSeconds);

        if (startPos >= startSample && startPos <= endSample) {
            double x = m_waveformRenderer->transformSamplePositionInRendererWorld(
                    startPos, ::WaveformRendererAbstract::Play);

            if (x >= 0 && x <= width) {
                painter.drawLine(QLineF(x, 0, x, height));

                if (m_style.showDiamonds) {
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(m_style.markerColor);
                    QPolygonF diamond;
                    diamond << QPointF(x, 4) << QPointF(x + 4, 0)
                            << QPointF(x, -4) << QPointF(x - 4, 0);
                    painter.drawPolygon(diamond);
                    painter.setPen(QPen(m_style.markerColor,
                            m_style.markerWidth,
                            m_style.markerLineStyle));
                }
            }
        }
    }
}

void WaveformRenderKeyCurve::drawKeyLabels(QPainter& painter, float width, float height) {
    Q_UNUSED(height);
    if (!m_showKeyLabels) {
        return;
    }
    if (m_segments.isEmpty()) {
        return;
    }

    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack) {
        return;
    }

    double trackLengthSeconds = pTrack->getDuration();
    double trackSamples = m_waveformRenderer->getTrackSamples();
    double startSample, endSample, dummy;
    getVisibleRange(startSample, endSample, dummy, dummy);

    QFont labelFont = painter.font();
    labelFont.setPointSize(m_style.fontSize);
    labelFont.setBold(true);
    painter.setFont(labelFont);

    const double minLabelSpacing = 100;
    double lastLabelX = -minLabelSpacing;
    const double labelY = 5;

    // Get user notation preference
    int notationValue = static_cast<int>(m_pKeyNotationCO->get());
    KeyUtils::KeyNotation notation = KeyUtils::keyNotationFromNumericValue(notationValue);

    for (const auto& seg : std::as_const(m_segments)) {
        if (seg.confidence < 50.0) {
            continue;
        }

        double startPos = seg.startTime * (trackSamples / trackLengthSeconds);

        if (startPos < startSample || startPos > endSample) {
            continue;
        }

        double x = m_waveformRenderer->transformSamplePositionInRendererWorld(
                startPos, ::WaveformRendererAbstract::Play);

        if (x >= 0 && x <= width && (x - lastLabelX) >= minLabelSpacing) {
            // Use keyId for transposition
            ChromaticKey baseKey = static_cast<ChromaticKey>(seg.keyId);
            int transposedKeyId = (static_cast<int>(baseKey) + m_currentTotalOffset) % 12;
            // Adjust for major/minor (keep the same mode)
            if (baseKey >= 13) { // minor keys start at 13
                transposedKeyId += 12;
            }

            QString transposedKey = KeyUtils::keyToString(
                    static_cast<ChromaticKey>(transposedKeyId), notation);

            QRect textRect = painter.fontMetrics().boundingRect(transposedKey);
            painter.setPen(QPen(getColorForKey(transposedKey), 1));
            painter.drawText(static_cast<int>(x + 5),
                    static_cast<int>(labelY + textRect.height()),
                    transposedKey);

            lastLabelX = x;
        }
    }
}

void WaveformRenderKeyCurve::drawWheelSlices(
        QPainter& painter, const QRectF& rect, const QRectF& innerRect) {
    // Draw outer slices (Majors)
    for (const auto& key : std::as_const(m_lancelotLayout)) {
        if (!key.isMinor) {
            double startAngle = key.angle;
            double endAngle = startAngle + 30;
            double spanAngle = endAngle - startAngle;
            int start = static_cast<int>(startAngle * 16);
            int span = static_cast<int>(spanAngle * 16);

            painter.setPen(QPen(QColor(200, 200, 200, 150), 1));
            painter.setBrush(Qt::NoBrush);
            painter.drawPie(rect, start, span);
        }
    }

    // Draw inner slices (Minors)
    for (const auto& key : std::as_const(m_lancelotLayout)) {
        if (key.isMinor) {
            double startAngle = key.angle;
            double endAngle = startAngle + 30;
            double spanAngle = endAngle - startAngle;
            int start = static_cast<int>(startAngle * 16);
            int span = static_cast<int>(spanAngle * 16);

            painter.setPen(QPen(QColor(200, 200, 200, 150), 1));
            painter.setBrush(Qt::NoBrush);
            painter.drawPie(innerRect, start, span);
        }
    }
}

void WaveformRenderKeyCurve::drawHighlightedSlice(
        QPainter& painter, const QRectF& rect, const QRectF& innerRect) {
    QString currentLancelotForDisplay = m_transposedLancelot.isEmpty()
            ? m_baseLancelot
            : m_transposedLancelot;
    if (currentLancelotForDisplay.isEmpty()) {
        return;
    }

    for (const auto& key : std::as_const(m_lancelotLayout)) {
        if (key.lancelot == currentLancelotForDisplay) {
            double startAngle = key.angle;
            int start = static_cast<int>(startAngle * 16);
            int span = static_cast<int>(30 * 16);

            QRectF drawRect = key.isMinor ? innerRect : rect;

            double time = m_animationTimer.elapsed() / 1000.0;
            int alpha = 100 + static_cast<int>(155 * sin(time * 8));

            painter.setPen(QPen(QColor(255, 255, 255, alpha), 2));
            painter.setBrush(QBrush(QColor(255, 255, 255, alpha / 2)));
            painter.drawPie(drawRect, start, span);
            break;
        }
    }
}

void WaveformRenderKeyCurve::drawWheelText(
        QPainter& painter, int wheelX, int wheelY, int wheelSize) {
    QString displayKey;

    // if (!m_transposedMusicalKey.isEmpty() && !m_transposedLancelot.isEmpty()) {
    //     displayKey = m_transposedMusicalKey + " (" + m_transposedLancelot + ")";
    // } else if (!m_currentWheelKey.isEmpty() && !m_baseLancelot.isEmpty()) {
    //     displayKey = m_currentWheelKey + " (" + m_baseLancelot + ")";
    // }

    if (!m_transposedMusicalKey.isEmpty() && !m_transposedLancelot.isEmpty()) {
        displayKey = m_transposedMusicalKey;
        if (!m_transposedMusicalKey.contains(m_transposedLancelot)) {
            displayKey += " (" + m_transposedLancelot + ")";
        }
    } else if (!m_currentWheelKey.isEmpty() && !m_baseLancelot.isEmpty()) {
        displayKey = m_currentWheelKey;
        if (!m_currentWheelKey.contains(m_baseLancelot)) {
            displayKey += " (" + m_baseLancelot + ")";
        }
    }

    if (displayKey.isEmpty()) {
        return;
    }

    QFont font = painter.font();
    int baseFontSize = static_cast<int>(m_waveformRenderer->getHeight() * 0.1);
    if (baseFontSize < 8) {
        baseFontSize = 8;
    }

    // Calculate text width and adjust font size if needed
    int maxWidth = wheelSize;
    int fontSize = baseFontSize;

    do {
        font.setPointSize(fontSize);
        painter.setFont(font);
        QRect textRect = painter.fontMetrics().boundingRect(displayKey);

        if (textRect.width() <= maxWidth || fontSize <= 6) {
            break;
        }
        fontSize--;
    } while (fontSize > 6);

    painter.setPen(QPen(QColor(255, 255, 255, 255), 1));
    painter.drawText(QRect(wheelX, wheelY + wheelSize + 3, wheelSize, 30),
            Qt::AlignCenter,
            displayKey);
}

void WaveformRenderKeyCurve::drawLancelotWheelComponents(
        QPainter& painter, float width, float height) {
    if (!m_showLancelotWheel) {
        return;
    }

    int wheelSize = static_cast<int>(height * 0.7);
    if (wheelSize < 60) {
        wheelSize = 60;
    }

    int wheelX = static_cast<int>(width - wheelSize - m_wheelMargin);
    int wheelY = m_wheelMargin;

    QRectF rect(wheelX, wheelY, wheelSize, wheelSize);
    QPointF center = rect.center();

    // Draw outer circle
    painter.setPen(QPen(QColor(150, 150, 150, 200), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(rect);

    // Draw inner circle
    double innerRadius = rect.width() * 0.35;
    QRectF innerRect(center.x() - innerRadius,
            center.y() - innerRadius,
            innerRadius * 2,
            innerRadius * 2);
    painter.setPen(QPen(QColor(150, 150, 150, 150), 1, Qt::DashLine));
    painter.drawEllipse(innerRect);

    drawWheelSlices(painter, rect, innerRect);
    drawHighlightedSlice(painter, rect, innerRect);
    drawWheelText(painter, wheelX, wheelY, wheelSize);
}

void WaveformRenderKeyCurve::calculateTransposedValues(int totalSemitones) {
    if (m_currentKeyId == 0) {
        if (showDebugAllshaderWaveformRenderKeyCurve) {
            qDebug() << "[WaveformRenderKeyCurve - Allshader] "
                        "calculateTransposedValues - no current key ID";
        }
        return;
    }

    // Get the current key's root note (0-11) and mode (major/minor)
    int rootNote = (m_currentKeyId - 1) % 12;
    bool isMinor = (m_currentKeyId >= 13);

    // Transpose the root note
    int transposedRoot = (rootNote + totalSemitones) % 12;
    if (transposedRoot < 0) {
        transposedRoot += 12;
    }

    // Calculate transposed keyId
    int transposedKeyId = transposedRoot + 1; // 1-12 for major
    if (isMinor) {
        transposedKeyId += 12; // 13-24 for minor
    }

    // Get Lancelot for the transposed key
    if (m_style.wheelType == KeyCurveStyle::WHEEL_MIXXX) {
        // Mixxx wheel - OpenKey notation
        static const int rootToOpenKey[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        int openKeyNumber = rootToOpenKey[transposedRoot];
        m_transposedLancelot = QString::number(openKeyNumber) + (isMinor ? "A" : "B");
    } else {
        // Standard Lancelot wheel
        static const int rootToLancelotMajor[12] = {8, 3, 10, 5, 12, 7, 2, 9, 4, 11, 6, 1};
        static const int rootToLancelotMinor[12] = {5, 12, 7, 2, 9, 4, 11, 6, 1, 8, 3, 10};

        int lancelotNumber;
        if (isMinor) {
            lancelotNumber = rootToLancelotMinor[transposedRoot];
        } else {
            lancelotNumber = rootToLancelotMajor[transposedRoot];
        }
        m_transposedLancelot = QString::number(lancelotNumber) + (isMinor ? "A" : "B");
    }

    // Get display key in user's preferred notation
    int notationValue = static_cast<int>(m_pKeyNotationCO->get());
    KeyUtils::KeyNotation notation = KeyUtils::keyNotationFromNumericValue(notationValue);
    m_transposedMusicalKey = KeyUtils::keyToString(
            static_cast<ChromaticKey>(transposedKeyId), notation);

    if (showDebugAllshaderWaveformRenderKeyCurve) {
        qDebug() << "[WaveformRenderKeyCurve - Allshader] calculateTransposedValues -"
                 << "original keyId:" << m_currentKeyId
                 << "transposed keyId:" << transposedKeyId
                 << "Lancelot:" << m_transposedLancelot
                 << "Display:" << m_transposedMusicalKey;
    }
}

QString WaveformRenderKeyCurve::normalizeKeyDisplay(const QString& key) const {
    QString normalized = key;
    normalized.replace(QChar(0x266D), 'b');
    normalized.replace(QChar(0x266F), '#');
    return normalized;
}

void WaveformRenderKeyCurve::updateKeyTextures() {
    // not used
}

QColor WaveformRenderKeyCurve::getColorForKey(const QString& key) const {
    int hash = 0;
    for (QChar ch : key) {
        hash += ch.unicode();
    }
    return QColor::fromHsv(hash % 360, 200, 200);
}

void WaveformRenderKeyCurve::createNode(const QImage& image) {
    if (m_pKeyCurveNode) {
        return;
    }

    m_pendingWheelImage = image;
    if (showDebugAllshaderWaveformRenderKeyCurve) {
        qDebug() << "[WaveformRenderKeyCurve - Allshader] Waiting for context to create node";
    }
}

void WaveformRenderKeyCurve::updateNode() {
    if (!m_pendingWheelImage.isNull()) {
        if (showDebugAllshaderWaveformRenderKeyCurve) {
            qDebug() << "[WaveformRenderKeyCurve - Allshader] Context available, creating node";
        }
        auto pNode = std::make_unique<WaveformKeyCurveNode>(
                m_waveformRenderer->getContext(), m_pendingWheelImage);
        m_pKeyCurveNode = pNode.get();
        m_pKeyCurveNodesParent->appendChildNode(std::move(pNode));
        m_pendingWheelImage = QImage();
    }

    if (!m_pKeyCurveNode) {
        return;
    }

    // Poll values
    if (m_pRateRatioCO) {
        m_currentRateRatio = m_pRateRatioCO->get();
    }
    if (m_pKeyControlCO) {
        m_currentKeyShift = m_pKeyControlCO->get();
    }
    if (m_pKeylockCO) {
        m_keylockEnabled = (m_pKeylockCO->get() > 0.5);
    }

    float width = static_cast<float>(m_waveformRenderer->getWidth());
    float height = static_cast<float>(m_waveformRenderer->getBreadth());
    if (width <= 0 || height <= 0) {
        return;
    }

    // Calculate current offset
    double pitchSemitones = m_keylockEnabled ? 0.0 : 12.0 * log2(m_currentRateRatio);
    int totalSemitones = static_cast<int>(std::round(pitchSemitones + m_currentKeyShift));
    m_currentTotalOffset = totalSemitones;

    calculateTransposedValues(totalSemitones);

    // Create image and draw
    QImage fullImage(static_cast<int>(width),
            static_cast<int>(height),
            QImage::Format_ARGB32_Premultiplied);
    fullImage.fill(Qt::transparent);

    QPainter painter(&fullImage);
    painter.setRenderHint(QPainter::Antialiasing);

    drawKeyMarkers(painter, width, height);
    drawKeyLabels(painter, width, height);
    drawLancelotWheelComponents(painter, width, height);

    painter.end();

    // Update texture
    auto texture = std::make_unique<rendergraph::Texture>(
            m_waveformRenderer->getContext(), fullImage);
    dynamic_cast<rendergraph::TextureMaterial&>(m_pKeyCurveNode->material())
            .setTexture(std::move(texture));
    m_pKeyCurveNode->markDirtyMaterial();

    // Update position
    float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
    m_pKeyCurveNode->updatePosition(0, 0, devicePixelRatio);
    m_pKeyCurveNode->markDirtyGeometry();
}

void WaveformRenderKeyCurve::update() {
    if (isSubtreeBlocked()) {
        return;
    }

    // Check if renderer size has changed
    float width = static_cast<float>(m_waveformRenderer->getWidth());
    float height = static_cast<float>(m_waveformRenderer->getBreadth());
    QSizeF currentRendererSize(width, height);

    bool sizeChanged = (currentRendererSize != m_previousRendererSize);
    if (sizeChanged) {
        m_previousRendererSize = currentRendererSize;
        if (showDebugAllshaderWaveformRenderKeyCurve) {
            qDebug() << "[WaveformRenderKeyCurve - Allshader] Renderer size changed to:"
                     << width << "x" << height;
        }

        if (m_pKeyCurveNode) {
            m_pKeyCurveNodesParent->removeChildNode(m_pKeyCurveNode);
            m_pKeyCurveNode = nullptr;
        }
        m_pendingWheelImage = QImage();
    }

    // Check if track changed
    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    TrackId currentTrackId = pTrack ? pTrack->getId() : TrackId();

    if (currentTrackId != m_lastTrackId) {
        m_lastTrackId = currentTrackId;

        // Clear all track-specific data
        m_segments.clear();
        m_currentKey.clear();
        m_currentWheelKey.clear();
        m_baseLancelot.clear();
        m_transposedLancelot.clear();
        m_transposedMusicalKey.clear();
        m_segmentsLoaded = false;

        m_currentLancelot.clear();
        m_transposedWheelKey.clear();
        m_trackLengthSeconds = 0.0;
        m_playPosition = 0.0;
        m_currentTotalOffset = 0;

        if (m_pKeyCurveNode) {
            m_pKeyCurveNodesParent->removeChildNode(m_pKeyCurveNode);
            m_pKeyCurveNode = nullptr;
        }
        m_pendingWheelImage = QImage();

        if (pTrack && pTrack->getId().isValid()) {
            if (showDebugAllshaderWaveformRenderKeyCurve) {
                qDebug() << "[WaveformRenderKeyCurve - Allshader] Loading new track";
            }
            updateCurrentKey();
        }
    }

    // if track is loaded -> Update play position and current key
    // if track had nog keysegments on load -> wait while analyzing
    // after 2 secs try loading segments again -> wheel will appear
    // if track is not playing but maybe re-analyzed -> check every 2 secs

    if (pTrack && m_pPlayPositionCO) {
        double newPosition = m_pPlayPositionCO->get();
        if (std::abs(newPosition - m_playPosition) > 0.001) {
            m_playPosition = newPosition;
            updateCurrentKey();
        } else if (m_reloadTimer.hasExpired(kReloadCheckIntervalMs)) {
            m_reloadTimer.restart();
            updateCurrentKey();
        }
    }

    // Create initial pending image if needed
    // Also recreate if size changed and we have segments
    if (!m_pKeyCurveNode && m_pendingWheelImage.isNull() && !m_segments.isEmpty()) {
        QImage wheelImage = createFullImage();
        createNode(wheelImage);
    } else if (sizeChanged && !m_segments.isEmpty() && m_pKeyCurveNode) {
        // If size changed and we already have a node, recreate it with new size
        QImage wheelImage = createFullImage();
        m_pKeyCurveNodesParent->removeChildNode(m_pKeyCurveNode);
        m_pKeyCurveNode = nullptr;
        createNode(wheelImage);
    }

    updateNode();
}

} // namespace allshader
