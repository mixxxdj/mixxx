#include "waveform/renderers/waveformrenderkeycurve.h"

#include <QDebug>
#include <QPainter>
#include <QSizeF>
#include <cmath>

#include "control/controlproxy.h"
#include "library/library_prefs.h"
#include "proto/keys.pb.h"
#include "skin/legacy/skincontext.h"
#include "track/keyutils.h"
#include "track/track.h"
#include "util/painterscope.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wskincolor.h"

using mixxx::track::io::key::ChromaticKey;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {
constexpr double WHEEL_START_ANGLE = 45.0;
constexpr double SLICE_ANGLE = 30.0;
constexpr int NUM_SLICES = 12;
constexpr bool showDebugWaveformRenderKeyCurve = false;
static constexpr int kReloadCheckIntervalMs = 2000;
} // namespace

WaveformRenderKeyCurve::WaveformRenderKeyCurve(WaveformWidgetRenderer* renderer)
        : WaveformRendererAbstract(renderer),
          m_pRateRatioCO(nullptr),
          m_pKeyControlCO(nullptr),
          m_pPlayPositionCO(nullptr),
          m_pKeylockCO(nullptr),
          m_pKeyNotationCO(nullptr),
          m_segments(),
          m_lancelotLayout(),
          m_style(),
          m_baseLancelot(),
          m_transposedLancelot(),
          m_transposedMusicalKey(),
          m_transposedWheelKey(),
          m_currentWheelKey(),
          m_lastLoadTime(),
          m_animationTimer(),
          m_reloadTimer(),
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
          m_previousRendererSize(0, 0) {
    m_animationTimer.start();
    m_reloadTimer.start();
    initLancelotLayout();

    m_pKeyNotationCO = std::make_unique<ControlProxy>(
            mixxx::library::prefs::kKeyNotationConfigKey);

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
}

void WaveformRenderKeyCurve::initPlayPositionControl() {
    if (!m_waveformRenderer || m_waveformRenderer->getGroup().isEmpty()) {
        if (showDebugWaveformRenderKeyCurve) {
            qDebug() << "[WaveformRenderKeyCurve] Cannot init playposition control - no group";
        }
        return;
    }

    if (showDebugWaveformRenderKeyCurve) {
        qDebug() << "[WaveformRenderKeyCurve] Creating playposition control for group:"
                 << m_waveformRenderer->getGroup();
    }

    m_pPlayPositionCO = std::make_unique<ControlProxy>(
            m_waveformRenderer->getGroup(), "playposition");

    m_currentPlayPosition = m_pPlayPositionCO->get();
    if (showDebugWaveformRenderKeyCurve) {
        qDebug() << "[WaveformRenderKeyCurve] Initial play position:" << m_currentPlayPosition;
    }
}

void WaveformRenderKeyCurve::initKeylockControl() {
    if (!m_waveformRenderer || m_waveformRenderer->getGroup().isEmpty()) {
        if (showDebugWaveformRenderKeyCurve) {
            qDebug() << "[WaveformRenderKeyCurve] Cannot init keylock control - no group";
        }
        return;
    }

    if (showDebugWaveformRenderKeyCurve) {
        qDebug() << "[WaveformRenderKeyCurve] Creating keylock control for group:"
                 << m_waveformRenderer->getGroup();
    }

    m_pKeylockCO = std::make_unique<ControlProxy>(
            m_waveformRenderer->getGroup(), "keylock");

    m_keylockEnabled = (m_pKeylockCO->get() > 0.5);
    if (showDebugWaveformRenderKeyCurve) {
        qDebug() << "[WaveformRenderKeyCurve] Initial keylock:" << m_keylockEnabled;
    }
}

void WaveformRenderKeyCurve::initRateRatioControl() {
    if (!m_waveformRenderer || m_waveformRenderer->getGroup().isEmpty()) {
        if (showDebugWaveformRenderKeyCurve) {
            qDebug() << "[WaveformRenderKeyCurve] Cannot init rate control - no group";
        }
        return;
    }

    if (showDebugWaveformRenderKeyCurve) {
        qDebug() << "[WaveformRenderKeyCurve] Creating rate ratio control for group:"
                 << m_waveformRenderer->getGroup();
    }

    m_pRateRatioCO = std::make_unique<ControlProxy>(
            m_waveformRenderer->getGroup(), "rate_ratio");

    m_currentRateRatio = m_pRateRatioCO->get();
    if (showDebugWaveformRenderKeyCurve) {
        qDebug() << "[WaveformRenderKeyCurve] Initial rate ratio:" << m_currentRateRatio;
    }
}

void WaveformRenderKeyCurve::initKeyControl() {
    if (!m_waveformRenderer || m_waveformRenderer->getGroup().isEmpty()) {
        if (showDebugWaveformRenderKeyCurve) {
            qDebug() << "[WaveformRenderKeyCurve] Cannot init key control - no group";
        }
        return;
    }

    if (showDebugWaveformRenderKeyCurve) {
        qDebug() << "[WaveformRenderKeyCurve] Creating key control for group:"
                 << m_waveformRenderer->getGroup();
    }

    m_pKeyControlCO = std::make_unique<ControlProxy>(
            m_waveformRenderer->getGroup(), "pitch");

    m_currentKeyShift = m_pKeyControlCO->get();
    if (showDebugWaveformRenderKeyCurve) {
        qDebug() << "[WaveformRenderKeyCurve] Initial key shift:" << m_currentKeyShift;
    }
}

void WaveformRenderKeyCurve::setup(const QDomNode& node, const SkinContext& skinContext) {
    // Text styling
    QString textColorStr = skinContext.selectString(node, QStringLiteral("KeyTextColor"));
    if (!textColorStr.isEmpty()) {
        m_style.textColor = QColor(textColorStr);
    }

    QString fontSizeStr = skinContext.selectString(node, QStringLiteral("KeyFontSize"));
    if (!fontSizeStr.isEmpty()) {
        m_style.fontSize = fontSizeStr.toInt();
    }

    QString fontOpacityStr = skinContext.selectString(node, QStringLiteral("KeyFontOpacity"));
    if (!fontOpacityStr.isEmpty()) {
        m_style.fontOpacity = fontOpacityStr.toInt();
    }
    m_style.textColor.setAlpha(m_style.fontOpacity);

    // Background styling
    QString bgColorStr = skinContext.selectString(node, QStringLiteral("KeyBackgroundColor"));
    if (!bgColorStr.isEmpty()) {
        m_style.backgroundColor = QColor(bgColorStr);
    }

    QString bgOpacityStr = skinContext.selectString(node, QStringLiteral("KeyBackgroundOpacity"));
    if (!bgOpacityStr.isEmpty()) {
        m_style.backgroundOpacity = bgOpacityStr.toInt();
    }
    m_style.backgroundColor.setAlpha(m_style.backgroundOpacity);

    // Marker styling
    QString markerColorStr = skinContext.selectString(node, QStringLiteral("KeyMarkerColor"));
    if (!markerColorStr.isEmpty()) {
        m_style.markerColor = QColor(markerColorStr);
    }

    QString markerWidthStr = skinContext.selectString(node, QStringLiteral("KeyMarkerWidth"));
    if (!markerWidthStr.isEmpty()) {
        m_style.markerWidth = markerWidthStr.toInt();
    }

    // Lancelot wheel visibility
    // QString showWheelStr = skinContext.selectString(node, QStringLiteral("ShowLancelotWheel"));
    // if (!showWheelStr.isEmpty()) {
    //    m_style.showLancelotWheel = (showWheelStr.compare("true", Qt::CaseInsensitive) == 0);
    //}

    QString wheelSizeStr = skinContext.selectString(node, QStringLiteral("LancelotWheelSize"));
    if (!wheelSizeStr.isEmpty()) {
        m_wheelSize = wheelSizeStr.toInt();
    }
}

void WaveformRenderKeyCurve::initLancelotLayout() {
    m_lancelotLayout.clear();

    QStringList majorLancelots = {
            "1B", "2B", "3B", "4B", "5B", "6B", "7B", "8B", "9B", "10B", "11B", "12B"};
    QStringList minorLancelots = {
            "1A", "2A", "3A", "4A", "5A", "6A", "7A", "8A", "9A", "10A", "11A", "12A"};

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

void WaveformRenderKeyCurve::onPlayPositionChanged(double value) {
    m_currentPlayPosition = value;
    updateCurrentWheelKey();
}

QString WaveformRenderKeyCurve::keyIdToLancelot(int keyId) const {
    bool isMinor = (keyId >= 13);
    int rootNote = (keyId - 1) % 12;

    int lancelotNumber;
    if (m_style.wheelType == KeyCurveStyle::WHEEL_MIXXX) {
        static const int rootToOpenKey[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        lancelotNumber = rootToOpenKey[rootNote];
    } else {
        static const int rootToCamelotMajor[12] = {8, 3, 10, 5, 12, 7, 2, 9, 4, 11, 6, 1};
        static const int rootToCamelotMinor[12] = {5, 12, 7, 2, 9, 4, 11, 6, 1, 8, 3, 10};
        lancelotNumber = isMinor ? rootToCamelotMinor[rootNote] : rootToCamelotMajor[rootNote];
    }

    return QString::number(lancelotNumber) + (isMinor ? "A" : "B");
}

void WaveformRenderKeyCurve::calculateTransposedValues(int totalSemitones) {
    if (m_currentKeyId == 0) {
        return;
    }

    int rootNote = (m_currentKeyId - 1) % 12;
    bool isMinor = (m_currentKeyId >= 13);

    int transposedRoot = (rootNote + totalSemitones) % 12;
    if (transposedRoot < 0) {
        transposedRoot += 12;
    }
    int transposedKeyId = transposedRoot + 1;
    if (isMinor) {
        transposedKeyId += 12;
    }

    // Get Lancelot for the transposed key
    if (m_style.wheelType == KeyCurveStyle::WHEEL_MIXXX) {
        static const int rootToOpenKey[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        int openKeyNumber = rootToOpenKey[transposedRoot];
        m_transposedLancelot = QString::number(openKeyNumber) + (isMinor ? "A" : "B");
    } else {
        static const int rootToCamelotMajor[12] = {8, 3, 10, 5, 12, 7, 2, 9, 4, 11, 6, 1};
        static const int rootToCamelotMinor[12] = {5, 12, 7, 2, 9, 4, 11, 6, 1, 8, 3, 10};
        int lancelotNumber = isMinor ? rootToCamelotMinor[transposedRoot]
                                     : rootToCamelotMajor[transposedRoot];
        m_transposedLancelot = QString::number(lancelotNumber) + (isMinor ? "A" : "B");
    }

    // Get display key in user's preferred notation
    int notationValue = static_cast<int>(m_pKeyNotationCO->get());
    KeyUtils::KeyNotation notation = KeyUtils::keyNotationFromNumericValue(notationValue);
    m_transposedMusicalKey = KeyUtils::keyToString(
            static_cast<ChromaticKey>(transposedKeyId), notation);
}

// Logic based on Loading KEY curve segments from DB
void WaveformRenderKeyCurve::updateCurrentWheelKey() {
    if (m_segments.isEmpty() || m_trackLengthSeconds <= 0) {
        return;
    }

    double positionSeconds = m_currentPlayPosition * m_trackLengthSeconds;
    int notationValue = static_cast<int>(m_pKeyNotationCO->get());
    KeyUtils::KeyNotation notation = KeyUtils::keyNotationFromNumericValue(notationValue);

    for (const auto& seg : std::as_const(m_segments)) {
        if (positionSeconds >= seg.startTime && positionSeconds <= seg.endTime) {
            QString displayKey = KeyUtils::keyToString(
                    static_cast<ChromaticKey>(seg.keyId), notation);
            if (m_currentWheelKey != displayKey) {
                m_currentWheelKey = displayKey;
                m_currentKeyId = seg.keyId;
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
        m_currentWheelKey = displayKey;
        m_currentKeyId = m_segments[0].keyId;
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

    if (showDebugWaveformRenderKeyCurve) {
        qDebug() << "[WaveformRenderKeyCurve] Keylock:" << m_keylockEnabled
                 << "Semitones:" << totalSemitones
                 << "Original:" << m_currentWheelKey << "(" << m_baseLancelot << ")"
                 << "-> Flashing:" << m_transposedLancelot
                 << "Display:" << m_transposedMusicalKey;
    }
}

void WaveformRenderKeyCurve::onSetTrack() {
    m_segments.clear();
    m_segmentsLoaded = false;
    m_currentKeyId = 0;
    m_currentWheelKey.clear();
    m_baseLancelot.clear();
    m_transposedLancelot.clear();
    m_transposedMusicalKey.clear();
    m_reloadTimer.restart();
    loadKeyCurve();
    initPlayPositionControl();
    initRateRatioControl();
    initKeyControl();
    initKeylockControl();
}

// Load key curve data from DB for the current track
void WaveformRenderKeyCurve::loadKeyCurve() {
    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack || !pTrack->getId().isValid()) {
        if (showDebugWaveformRenderKeyCurve) {
            qDebug() << "[WaveformRenderKeyCurve] No valid track";
        }
        return;
    }

    m_trackLengthSeconds = pTrack->getDuration();

    if (showDebugWaveformRenderKeyCurve) {
        qDebug() << "[WaveformRenderKeyCurve] Loading Key curve for track:"
                 << pTrack->getTitle()
                 << "ID:" << pTrack->getId().toString();
    }

    // Load from database via Track object
    QList<KeySegmentsPointer> segments = pTrack->getKeySegments();

    if (segments.isEmpty()) {
        if (showDebugWaveformRenderKeyCurve) {
            qDebug() << "[WaveformRenderKeyCurve] No key segments in database for track:"
                     << pTrack->getTitle()
                     << "ID:" << pTrack->getId().toString();
        }
        // Clear existing segments if any (in case they were reloaded)
        if (!m_segments.isEmpty()) {
            m_segments.clear();
            m_currentWheelKey.clear();
            m_baseLancelot.clear();
            m_currentKeyId = 0;
        }
        return;
    }

    // Convert KeySegments to KeySegment format
    m_segments.clear();
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

    m_segmentsLoaded = true;

    if (showDebugWaveformRenderKeyCurve) {
        qDebug() << "[WaveformRenderKeyCurve] Loaded" << m_segments.size()
                 << "key segments from database for track:"
                 << pTrack->getTitle()
                 << "ID:" << pTrack->getId().toString();
        // 1st segment as check
        if (!m_segments.isEmpty()) {
            qDebug() << "[WaveformRenderKeyCurve] First segment - keyId:" << m_segments[0].keyId
                     << "keyText:" << m_segments[0].key;
        }
    }

    // Set initial wheel key
    if (!m_segments.isEmpty()) {
        int notationValue = static_cast<int>(m_pKeyNotationCO->get());
        KeyUtils::KeyNotation notation = KeyUtils::keyNotationFromNumericValue(notationValue);
        m_currentWheelKey = KeyUtils::keyToString(
                static_cast<ChromaticKey>(m_segments[0].keyId), notation);
        m_currentKeyId = m_segments[0].keyId;
        m_baseLancelot = keyIdToLancelot(m_segments[0].keyId);
        updateTransposedKey();
    }
}

QString WaveformRenderKeyCurve::normalizeKeyDisplay(const QString& key) const {
    QString normalized = key;
    normalized.replace(QChar(0x266D), 'b');
    normalized.replace(QChar(0x266F), '#');
    return normalized;
}

void WaveformRenderKeyCurve::drawWheelSlice(QPainter* painter,
        const QRectF& rect,
        double startAngle,
        double endAngle,
        const QString& lancelot,
        bool isMinor) {
    Q_UNUSED(lancelot);
    Q_UNUSED(isMinor);
    double spanAngle = endAngle - startAngle;
    if (spanAngle <= 0) {
        return;
    }

    int start = static_cast<int>(startAngle * 16);
    int span = static_cast<int>(spanAngle * 16);

    painter->setPen(QPen(QColor(200, 200, 200, 150), 1));
    painter->setBrush(Qt::NoBrush);
    painter->drawPie(rect, start, span);
}

void WaveformRenderKeyCurve::drawHighlightedWheelKey(QPainter* painter,
        const QRectF& outerRect,
        const QRectF& innerRect,
        const QString& lancelot) {
    if (lancelot.isEmpty()) {
        return;
    }

    for (const auto& key : std::as_const(m_lancelotLayout)) {
        if (key.lancelot == lancelot) {
            double startAngle = key.angle;
            int start = static_cast<int>(startAngle * 16);
            int span = static_cast<int>(30 * 16);

            QRectF drawRect = key.isMinor ? innerRect : outerRect;

            double time = m_animationTimer.elapsed() / 1000.0;
            int alpha = 100 + static_cast<int>(155 * sin(time * 8));

            painter->setPen(QPen(QColor(255, 255, 255, alpha), 2));
            painter->setBrush(QBrush(QColor(255, 255, 255, alpha / 2)));
            painter->drawPie(drawRect, start, span);
            break;
        }
    }
}

void WaveformRenderKeyCurve::drawLancelotWheel(QPainter* painter) {
    if (!m_showLancelotWheel) {
        return;
    }
    if (m_segments.isEmpty()) {
        return;
    }

    m_wheelSize = static_cast<int>(m_waveformRenderer->getHeight() * 0.7);
    if (m_wheelSize < 60) {
        m_wheelSize = 60;
    }

    int x = m_waveformRenderer->getWidth() - m_wheelSize - m_wheelMargin;
    int y = m_wheelMargin;
    QRectF rect(x, y, m_wheelSize, m_wheelSize);
    QPointF center = rect.center();

    // Draw outer circle
    painter->setPen(QPen(QColor(150, 150, 150, 200), 2));
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(rect);

    // Draw inner circle
    double innerRadius = rect.width() * 0.35;
    QRectF innerRect(center.x() - innerRadius,
            center.y() - innerRadius,
            innerRadius * 2,
            innerRadius * 2);
    painter->setPen(QPen(QColor(150, 150, 150, 150), 1, Qt::DashLine));
    painter->drawEllipse(innerRect);

    // Draw outer slices (Majors)
    for (const auto& key : std::as_const(m_lancelotLayout)) {
        if (!key.isMinor) {
            drawWheelSlice(painter, rect, key.angle, key.angle + 30, key.lancelot, false);
        }
    }

    // Draw inner slices (Minors)
    for (const auto& key : std::as_const(m_lancelotLayout)) {
        if (key.isMinor) {
            drawWheelSlice(painter, innerRect, key.angle, key.angle + 30, key.lancelot, true);
        }
    }

    // Draw highlighted key using transposed Lancelot
    if (!m_transposedLancelot.isEmpty()) {
        drawHighlightedWheelKey(painter, rect, innerRect, m_transposedLancelot);
    } else if (!m_baseLancelot.isEmpty()) {
        drawHighlightedWheelKey(painter, rect, innerRect, m_baseLancelot);
    }

    // Display transposed musical key with its Lancelot number
    QString displayKey;
    // if (!m_transposedMusicalKey.isEmpty() && !m_transposedLancelot.isEmpty()) {
    //     displayKey = normalizeKeyDisplay(m_transposedMusicalKey) + " (" +
    //             m_transposedLancelot + ")";
    // } else if (!m_currentWheelKey.isEmpty() && !m_baseLancelot.isEmpty()) {
    //     displayKey = normalizeKeyDisplay(m_currentWheelKey) + " (" + m_baseLancelot + ")";
    // }
    if (!m_transposedMusicalKey.isEmpty() && !m_transposedLancelot.isEmpty()) {
        QString normalizedKey = normalizeKeyDisplay(m_transposedMusicalKey);
        displayKey = normalizedKey;
        if (!normalizedKey.contains(m_transposedLancelot)) {
            displayKey += " (" + m_transposedLancelot + ")";
        }
    } else if (!m_currentWheelKey.isEmpty() && !m_baseLancelot.isEmpty()) {
        QString normalizedKey = normalizeKeyDisplay(m_currentWheelKey);
        displayKey = normalizedKey;
        if (!normalizedKey.contains(m_baseLancelot)) {
            displayKey += " (" + m_baseLancelot + ")";
        }
    }

    if (!displayKey.isEmpty()) {
        QFont font = painter->font();
        int baseFontSize = static_cast<int>(m_waveformRenderer->getHeight() * 0.1);
        if (baseFontSize < 8) {
            baseFontSize = 8;
        }

        int fontSize = baseFontSize;
        do {
            font.setPointSize(fontSize);
            painter->setFont(font);
            QRect textRect = painter->fontMetrics().boundingRect(displayKey);
            if (textRect.width() <= m_wheelSize || fontSize <= 6) {
                break;
            }
            fontSize--;
        } while (fontSize > 6);

        painter->setPen(QPen(QColor(255, 255, 255, 255), 1));
        painter->drawText(QRect(x, y + m_wheelSize + 3, m_wheelSize, 30),
                Qt::AlignCenter,
                displayKey);
    }
}

QColor WaveformRenderKeyCurve::getColorForKey(const QString& key) const {
    int hash = 0;
    for (QChar ch : key) {
        hash += ch.unicode();
    }
    return QColor::fromHsv(hash % 360, 200, 200);
}

void WaveformRenderKeyCurve::drawKeyLabel(QPainter* painter,
        const QPointF& position,
        const QString& key,
        Qt::Orientation orientation) {
    QColor keyColor = getColorForKey(key);
    keyColor.setAlpha(m_style.fontOpacity);

    QFont labelFont = painter->font();
    labelFont.setPointSize(m_style.fontSize);
    labelFont.setBold(true);
    painter->setFont(labelFont);

    QRect textRect = painter->fontMetrics().boundingRect(key);
    // if background needed
    // int padding = 4;
    int labelHeight = textRect.height();

    QPointF labelPos;
    if (orientation == Qt::Horizontal) {
        labelPos = QPointF(position.x() + 5, position.y());

        if (labelPos.x() + textRect.width() > painter->device()->width()) {
            labelPos.setX(position.x() - textRect.width() - 5);
        }

        if (labelPos.x() < 0) {
            labelPos.setX(0);
        }

        if (m_style.showBackground) {
            // if background needed
            // QRect bgRect(static_cast<int>(labelPos.x() - padding),
            //         static_cast<int>(labelPos.y() - padding),
            //         textRect.width() + padding * 2,
            //         labelHeight + padding * 2);
            // painter->setPen(Qt::NoPen);
            // painter->setBrush(m_style.backgroundColor);

            // painter->drawRoundedRect(bgRect, 3, 3);
        }

        painter->setPen(QPen(keyColor, 1));
        painter->drawText(static_cast<int>(labelPos.x()),
                static_cast<int>(labelPos.y() + labelHeight),
                key);
    }
}

void WaveformRenderKeyCurve::drawKeyChangeLabel(QPainter* painter,
        const QPointF& position,
        const QString& previousKey,
        const QString& currentKey,
        Qt::Orientation orientation) {
    Q_UNUSED(previousKey);
    drawKeyLabel(painter, position, currentKey, orientation);
}

void WaveformRenderKeyCurve::draw(QPainter* painter, QPaintEvent* /*event*/) {
    // Check if renderer size has changed
    const float rendererWidth = m_waveformRenderer->getWidth();
    const float rendererHeight = m_waveformRenderer->getHeight();
    const Qt::Orientation orientation = m_waveformRenderer->getOrientation();

    QSizeF currentRendererSize(rendererWidth, rendererHeight);

    bool sizeChanged = (currentRendererSize != m_previousRendererSize);
    if (sizeChanged) {
        m_previousRendererSize = currentRendererSize;
        if (showDebugWaveformRenderKeyCurve) {
            qDebug() << "[WaveformRenderKeyCurve] Renderer size changed to:"
                     << rendererWidth << "x" << rendererHeight;
        }
        if (m_waveformRenderer) {
            QWidget* widget = dynamic_cast<QWidget*>(m_waveformRenderer);
            if (widget) {
                widget->update();
            }
        }
    }

    // if track is loaded -> Update play position and current key
    // if track had no keysegments on load -> wait while analyzing
    // after 2 secs try loading segments again -> wheel will appear
    // if track is not playing but maybe re-analyzed -> check every 2 secs
    if (m_reloadTimer.hasExpired(kReloadCheckIntervalMs)) {
        m_reloadTimer.restart();
        // Only reload if we don't have segments or if track changed
        if (m_segments.isEmpty()) {
            if (showDebugWaveformRenderKeyCurve) {
                qDebug() << "[WaveformRenderKeyCurve] Periodic reload check - loading segments";
            }
            loadKeyCurve();
        }
    }

    if (!m_visible || m_segments.isEmpty()) {
        drawLancelotWheel(painter);
        return;
    }

    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack) {
        drawLancelotWheel(painter);
        return;
    }

    if (m_pPlayPositionCO) {
        double newPlayPosition = m_pPlayPositionCO->get();
        if (std::abs(newPlayPosition - m_currentPlayPosition) > 0.001) {
            m_currentPlayPosition = newPlayPosition;
            onPlayPositionChanged(m_currentPlayPosition);
        }
    }

    if (m_pRateRatioCO) {
        double newRate = m_pRateRatioCO->get();
        if (std::abs(newRate - m_currentRateRatio) > 0.001) {
            m_currentRateRatio = newRate;
            updateTransposedKey();
        }
    }

    if (m_pKeyControlCO) {
        double newShift = m_pKeyControlCO->get();
        if (std::abs(newShift - m_currentKeyShift) > 0.001) {
            m_currentKeyShift = newShift;
            updateTransposedKey();
        }
    }

    if (m_pKeylockCO) {
        double newKeylock = m_pKeylockCO->get();
        bool newEnabled = (newKeylock > 0.5);
        if (m_keylockEnabled != newEnabled) {
            m_keylockEnabled = newEnabled;
            if (showDebugWaveformRenderKeyCurve) {
                qDebug() << "[WaveformRenderKeyCurve] Keylock changed to:" << m_keylockEnabled;
            }
            updateTransposedKey();
        }
    }

    double trackLengthSeconds = pTrack->getDuration();
    double trackSamples = m_waveformRenderer->getTrackSamples();

    if (trackLengthSeconds <= 0 || trackSamples <= 0) {
        drawLancelotWheel(painter);
        return;
    }

    double firstDisplayedPosition = m_waveformRenderer->getFirstDisplayedPosition();
    double lastDisplayedPosition = m_waveformRenderer->getLastDisplayedPosition();

    firstDisplayedPosition = qMax(0.0, firstDisplayedPosition);
    lastDisplayedPosition = qMin(1.0, lastDisplayedPosition);

    if (firstDisplayedPosition >= lastDisplayedPosition) {
        firstDisplayedPosition = 0.0;
        lastDisplayedPosition = 1.0;
    }

    double startSample = firstDisplayedPosition * trackSamples;
    double endSample = lastDisplayedPosition * trackSamples;

    PainterScope scope(painter);
    painter->setRenderHint(QPainter::Antialiasing);

    const double labelY = 5;
    const double minLabelSpacing = 100;

    // Draw key markers
    if (m_showKeyMarkers) {
        painter->setPen(QPen(m_style.markerColor, m_style.markerWidth, m_style.markerLineStyle));
        for (const auto& seg : std::as_const(m_segments)) {
            // double startTime = seg.startTime;
            double startPos = seg.startTime * (trackSamples / trackLengthSeconds);

            if (startPos >= startSample && startPos <= endSample) {
                double x = m_waveformRenderer->transformSamplePositionInRendererWorld(
                        startPos, ::WaveformRendererAbstract::Play);

                if (x >= 0 && x <= rendererWidth) {
                    if (orientation == Qt::Horizontal) {
                        painter->drawLine(QLineF(x, 0, x, rendererHeight));
                    }
                }
            }
        }
    }

    // Draw key labels
    if (m_showKeyLabels) {
        QFont labelFont = painter->font();
        labelFont.setPointSize(m_style.fontSize);
        labelFont.setBold(true);
        painter->setFont(labelFont);

        double lastLabelX = -minLabelSpacing;
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

            if (x >= 0 && x <= rendererWidth && (x - lastLabelX) >= minLabelSpacing) {
                if (orientation == Qt::Horizontal) {
                    // Use keyId for transposition
                    ChromaticKey baseKey = static_cast<ChromaticKey>(seg.keyId);
                    int transposedKeyId = (static_cast<int>(baseKey) + m_currentTotalOffset) % 12;
                    if (baseKey >= 13) {
                        transposedKeyId += 12;
                    }

                    QString transposedKey = KeyUtils::keyToString(
                            static_cast<ChromaticKey>(transposedKeyId),
                            notation);
                    drawKeyLabel(painter, QPointF(x, labelY), transposedKey, orientation);
                    lastLabelX = x;
                }
            }
        }
    }
    if (m_showLancelotWheel) {
        drawLancelotWheel(painter);
    }
}
