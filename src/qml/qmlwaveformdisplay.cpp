#include "qml/qmlwaveformdisplay.h"

#include <QQuickWindow>
#include <QSGFlatColorMaterial>
#include <QSGSimpleRectNode>
#include <QSGVertexColorMaterial>
#include <cmath>

#include "mixer/basetrackplayer.h"
#include "moc_qmlwaveformdisplay.cpp"
#include "qml/qmlplayerproxy.h"
#include "waveform/renderers/waveformdisplayrange.h"

namespace {

// float to fixed point with 8 fractional bits, clipped at 4.0
uint32_t toFrac8(float x) {
    return std::min<uint32_t>(static_cast<uint32_t>(std::max(x, 0.f) * 256.f), 4 * 256);
}

constexpr size_t frac16sqrtTableSize{(3 * 4 * 255 * 256) / 16 + 1};

// scaled sqrt lookable table to convert maxAll and maxAllNext as calculated
// in updatePaintNode back to y coordinates
class Frac16SqrtTableSingleton {
  public:
    static Frac16SqrtTableSingleton& getInstance() {
        static Frac16SqrtTableSingleton instance;
        return instance;
    }

    inline float get(uint32_t x) const {
        // The maximum value of fact16x can be (as uint32_t) 3 * 4 * 255 * 256,
        // which would be exessive for the table size. We divide by 16 in order
        // to get a more reasonable size.
        return m_table[x >> 4];
    }

  private:
    float* m_table;
    Frac16SqrtTableSingleton()
            : m_table(new float[frac16sqrtTableSize]) {
        // In the original implementation, the result of sqrt(maxAll) is divided
        // by sqrt(3 * 255 * 255);
        // We get rid of that division and bake it into this table.
        // Additionally, we divide the index for the lookup by 16 (see get(...)),
        // so we need to invert that here.
        const float f = (3.f * 255.f * 255.f / 16.f);
        for (uint32_t i = 0; i < frac16sqrtTableSize; i++) {
            m_table[i] = std::sqrt(static_cast<float>(i) / f);
        }
    }
    ~Frac16SqrtTableSingleton() {
        delete[] m_table;
    }
    Frac16SqrtTableSingleton(const Frac16SqrtTableSingleton&) = delete;
    Frac16SqrtTableSingleton& operator=(const Frac16SqrtTableSingleton&) = delete;
};

inline float frac16_sqrt(uint32_t x) {
    return Frac16SqrtTableSingleton::getInstance().get(x);
}

} // namespace

namespace mixxx {
namespace qml {

QmlWaveformDisplay::QmlWaveformDisplay(QQuickItem* parent)
        : QQuickItem(parent),
          m_pPlayer(nullptr) {
    Frac16SqrtTableSingleton::getInstance(); // initializes table

    setFlag(QQuickItem::ItemHasContents, true);

    connect(this, &QmlWaveformDisplay::windowChanged, this, &QmlWaveformDisplay::slotWindowChanged);
}

QmlWaveformDisplay::~QmlWaveformDisplay() {
    disconnect(this);
    delete m_pWaveformDisplayRange;
}

void QmlWaveformDisplay::slotWindowChanged(QQuickWindow* window) {
    connect(window, &QQuickWindow::frameSwapped, this, &QmlWaveformDisplay::slotFrameSwapped);
    m_timer.restart();
}

int QmlWaveformDisplay::fromTimerToNextSyncMicros(const PerformanceTimer& timer) {
    // TODO @m0dB probably better to use a singleton instead of deriving QmlWaveformDisplay from
    // ISyncTimeProvider and have each keep track of this.
    int difference = static_cast<int>(m_timer.difference(timer).toIntegerMicros());
    // int math is fine here, because we do not expect times > 4.2 s

    return difference + m_syncIntervalTimeMicros;
}

void QmlWaveformDisplay::slotFrameSwapped() {
    m_timer.restart();
}

inline uint32_t frac8Pow2ToFrac16(uint32_t x) {
    // x is the result of multiplying two fixedpoint values with 8 fraction bits,
    // thus x has 16 fraction bits, which is also what we want to return for this
    // function. We would naively return (x * x) >> 16, but x * x would overflow
    // the 32 bits for values > 1, so we shift before multiplying.
    x >>= 8;
    return (x * x);
}

inline uint32_t math_max_u32(uint32_t a, uint32_t b) {
    return std::max(a, b);
}

inline uint32_t math_max_u32(uint32_t a, uint32_t b, uint32_t c) {
    return std::max(a, std::max(b, c));
}

QSGNode* QmlWaveformDisplay::updatePaintNode(QSGNode* old, QQuickItem::UpdatePaintNodeData*) {
    // TODO @m0dB make members and property
    uint32_t m_rgbLowColor_r = 255;
    uint32_t m_rgbMidColor_r = 0;
    uint32_t m_rgbHighColor_r = 0;
    uint32_t m_rgbLowColor_g = 0;
    uint32_t m_rgbMidColor_g = 255;
    uint32_t m_rgbHighColor_g = 0;
    uint32_t m_rgbLowColor_b = 0;
    uint32_t m_rgbMidColor_b = 0;
    uint32_t m_rgbHighColor_b = 255;

    auto* clipNode = dynamic_cast<QSGClipNode*>(old);

    if (m_pWaveformDisplayRange == nullptr) {
        return clipNode;
    }

    if (!m_pCurrentTrack) {
        return clipNode;
    }

    ConstWaveformPointer waveform = m_pCurrentTrack->getWaveform();
    if (waveform.isNull()) {
        return clipNode;
    }

    const int dataSize = waveform->getDataSize();
    if (dataSize <= 1) {
        return clipNode;
    }

    const WaveformData* data = waveform->data();
    if (data == nullptr) {
        return clipNode;
    }

    m_pWaveformDisplayRange->resize(static_cast<int>(width() * window()->devicePixelRatio()),
            static_cast<int>(height() * window()->devicePixelRatio()),
            window()->devicePixelRatio());
    // this as ISyncTimeProvider
    m_pWaveformDisplayRange->onPreRender(this);

    QSGGeometry* geometry;
    QSGGeometryNode* geometryNode;
    QSGSimpleRectNode* bgNode;
    if (!clipNode) {
        clipNode = new QSGClipNode();
        geometryNode = new QSGGeometryNode();
        geometry = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), 0);
        geometryNode->setGeometry(geometry);
        geometryNode->setFlag(QSGNode::OwnsGeometry);

        auto material = new QSGVertexColorMaterial();
        geometry->setDrawingMode(QSGGeometry::DrawLines);
        geometryNode->setMaterial(material);
        geometryNode->setFlag(QSGNode::OwnsMaterial);

        bgNode = new QSGSimpleRectNode();
        bgNode->appendChildNode(geometryNode);
        // TODO @m0dB make property
        bgNode->setColor(Qt::black);

        clipNode->appendChildNode(bgNode);
    } else {
        bgNode = dynamic_cast<QSGSimpleRectNode*>(clipNode->childAtIndex(0));
        geometryNode = dynamic_cast<QSGGeometryNode*>(bgNode->childAtIndex(0));
        geometry = geometryNode->geometry();
    }
    bgNode->setRect(boundingRect());
    clipNode->setClipRect(boundingRect());
    clipNode->setIsRectangular(true);

    const float devicePixelRatio = m_pWaveformDisplayRange->getDevicePixelRatio();
    const float invDevicePixelRatio = 1.f / devicePixelRatio;

    const int length = static_cast<int>(static_cast<float>(m_pWaveformDisplayRange->getLength()));

    // length waveform lines, 1 reference line, 2 points per line
    geometry->allocate((length + 1) * 2);

    QSGGeometry::ColoredPoint2D* vertices = geometry->vertexDataAsColoredPoint2D();

    const double firstVisualIndex = m_pWaveformDisplayRange->getFirstDisplayedPosition() * dataSize;
    const double lastVisualIndex = m_pWaveformDisplayRange->getLastDisplayedPosition() * dataSize;

    // Represents the # of waveform data points per horizontal pixel.
    const double visualIncrementPerPixel =
            (lastVisualIndex - firstVisualIndex) / static_cast<double>(length);

    // Per-band gain from the EQ knobs.
    float allGain(1.0), lowGain(1.0), midGain(1.0), highGain(1.0);
    // getGains(&allGain, &lowGain, &midGain, &highGain);

    // gains in 8 bit fractional fixed point
    const uint32_t frac8LowGain(toFrac8(lowGain));
    const uint32_t frac8MidGain(toFrac8(midGain));
    const uint32_t frac8HighGain(toFrac8(highGain));

    const float breadth =
            static_cast<float>(m_pWaveformDisplayRange->getBreadth()) /
            devicePixelRatio;
    const float halfBreadth = breadth / 2.0f;

    const float heightFactor = allGain * halfBreadth;

    // Effective visual index of x
    double xVisualSampleIndex = firstVisualIndex;

    int vertexIndex = 0;

    QColor axesColor(255, 255, 255, 255); // TODO @m0dB
    vertices[vertexIndex++].set(0.f,
            halfBreadth,
            axesColor.red(),
            axesColor.green(),
            axesColor.blue(),
            axesColor.alpha());
    vertices[vertexIndex++].set(static_cast<float>(length / devicePixelRatio),
            halfBreadth,
            axesColor.red(),
            axesColor.green(),
            axesColor.blue(),
            axesColor.alpha());

    for (int pos = 0; pos < length; ++pos) {
        // Our current pixel (x) corresponds to a number of visual samples
        // (visualSamplerPerPixel) in our waveform object. We take the max of
        // all the data points on either side of xVisualSampleIndex within a
        // window of 'maxSamplingRange' visual samples to measure the maximum
        // data point contained by this pixel.
        double maxSamplingRange = visualIncrementPerPixel / 2.0;

        // Since xVisualSampleIndex is in visual-samples (e.g. R,L,R,L) we want
        // to check +/- maxSamplingRange frames, not samples. To do this, divide
        // xVisualSampleIndex by 2. Since frames indices are integers, we round
        // to the nearest integer by adding 0.5 before casting to int.
        int visualFrameStart = int(xVisualSampleIndex / 2.0 - maxSamplingRange + 0.5);
        int visualFrameStop = int(xVisualSampleIndex / 2.0 + maxSamplingRange + 0.5);
        const int lastVisualFrame = dataSize / 2 - 1;

        // We now know that some subset of [visualFrameStart, visualFrameStop]
        // lies within the valid range of visual frames. Clamp
        // visualFrameStart/Stop to within [0, lastVisualFrame].
        visualFrameStart = math_clamp(visualFrameStart, 0, lastVisualFrame);
        visualFrameStop = math_clamp(visualFrameStop, 0, lastVisualFrame);

        int visualIndexStart = visualFrameStart * 2;
        int visualIndexStop = visualFrameStop * 2;

        visualIndexStart = std::max(visualIndexStart, 0);
        visualIndexStop = std::min(visualIndexStop, dataSize);

        uint32_t maxLow = 0;
        uint32_t maxMid = 0;
        uint32_t maxHigh = 0;

        uint32_t maxAll[2] = {0, 0};

        for (int chn = 0; chn < 2; chn++) {
            // data is interleaved left / right
            for (int i = visualIndexStart + chn; i < visualIndexStop + chn; i += 2) {
                const WaveformData& waveformData = data[i];

                maxLow = math_max_u32(maxLow, waveformData.filtered.low);
                maxMid = math_max_u32(maxMid, waveformData.filtered.mid);
                maxHigh = math_max_u32(maxHigh, waveformData.filtered.high);

                uint32_t all = frac8Pow2ToFrac16(waveformData.filtered.low * frac8LowGain) +
                        frac8Pow2ToFrac16(waveformData.filtered.mid * frac8MidGain) +
                        frac8Pow2ToFrac16(waveformData.filtered.high * frac8HighGain);
                maxAll[chn] = math_max(maxAll[chn], all);
            }
        }

        // We can do these integer calculation safely, staying well within the
        // 32 bit range, and we will normalize below.
        maxLow *= frac8LowGain;
        maxMid *= frac8MidGain;
        maxHigh *= frac8HighGain;
        uint32_t red = maxLow * m_rgbLowColor_r + maxMid * m_rgbMidColor_r +
                maxHigh * m_rgbHighColor_r;
        uint32_t green = maxLow * m_rgbLowColor_g + maxMid * m_rgbMidColor_g +
                maxHigh * m_rgbHighColor_g;
        uint32_t blue = maxLow * m_rgbLowColor_b + maxMid * m_rgbMidColor_b +
                maxHigh * m_rgbHighColor_b;

        // Normalize red, green, blue to 0..255, using the maximum of the three and
        // this fixed point arithmetic trick:
        // max / ((max>>8)+1) = 0..255
        uint32_t max = math_max_u32(red, green, blue);
        max >>= 8;

        if (max == 0) {
            // avoid division by 0
            red = 0;
            green = 0;
            blue = 0;
        } else {
            max++; // important, otherwise we normalize to 256

            red /= max;
            green /= max;
            blue /= max;
        }

        const float fpos = static_cast<float>(pos) * invDevicePixelRatio;
        vertices[vertexIndex++].set(fpos,
                halfBreadth - heightFactor * frac16_sqrt(maxAll[0]),
                red,
                green,
                blue,
                255);
        vertices[vertexIndex++].set(fpos,
                halfBreadth + heightFactor * frac16_sqrt(maxAll[1]),
                red,
                green,
                blue,
                255);

        xVisualSampleIndex += visualIncrementPerPixel;
    }

    geometryNode->markDirty(QSGNode::DirtyGeometry);
    update();

    return clipNode;
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

    emit playerChanged(m_pPlayer);

    update();
}

void QmlWaveformDisplay::setGroup(const QString& group) {
    if (m_group == group) {
        return;
    }

    m_group = group;
    emit groupChanged(group);

    // TODO m0dB unique_ptr ?
    delete m_pWaveformDisplayRange;
    m_pWaveformDisplayRange = new WaveformDisplayRange(m_group);
    m_pWaveformDisplayRange->init();
}

const QString& QmlWaveformDisplay::getGroup() const {
    return m_group;
}

void QmlWaveformDisplay::slotTrackLoaded(TrackPointer pTrack) {
    // TODO: Investigate if it's a bug that this debug assertion fails when
    // passing tracks on the command line
    // DEBUG_ASSERT(m_pCurrentTrack == pTrack);
    setCurrentTrack(pTrack);
}

void QmlWaveformDisplay::slotTrackLoading(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pOldTrack); // only used in DEBUG_ASSERT
    DEBUG_ASSERT(m_pCurrentTrack == pOldTrack);
    setCurrentTrack(pNewTrack);
}

void QmlWaveformDisplay::slotTrackUnloaded() {
    setCurrentTrack(nullptr);
}

void QmlWaveformDisplay::setCurrentTrack(TrackPointer pTrack) {
    // TODO: Check if this is actually possible
    if (m_pCurrentTrack == pTrack) {
        return;
    }

    if (m_pCurrentTrack != nullptr) {
        disconnect(m_pCurrentTrack.get(), nullptr, this, nullptr);
    }

    m_pCurrentTrack = pTrack;
    if (pTrack != nullptr) {
        connect(pTrack.get(),
                &Track::waveformSummaryUpdated,
                this,
                &QmlWaveformDisplay::slotWaveformUpdated);
    }
    slotWaveformUpdated();

    if (m_pWaveformDisplayRange) {
        m_pWaveformDisplayRange->setTrack(m_pCurrentTrack);
    }
}

void QmlWaveformDisplay::slotWaveformUpdated() {
    update();
}

} // namespace qml
} // namespace mixxx
