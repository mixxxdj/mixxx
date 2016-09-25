#include "waveform/overviews/overviewrenderthread.h"

#include <QPainter>

#include "library/dao/analysisdao.h"
#include "moc_overviewrenderthread.cpp"
#include "util/colorcomponents.h"
#include "util/db/dbconnectionpooled.h"
#include "util/db/dbconnectionpooler.h"
#include "util/logger.h"
#include "util/math.h"
#include "util/timer.h"
#include "waveform/overviews/overviewtype.h"
#include "waveform/waveformfactory.h"

mixxx::Logger kLogger("OverviewRenderThread");

RenderTrackOverview::RenderTrackOverview(TrackId trackId,
        mixxx::OverviewType type,
        WaveformSignalColors signalColors)
        : m_trackId(trackId),
          m_type(type),
          m_signalColors(signalColors) {
}

TrackId RenderTrackOverview::getTrackId() const {
    return m_trackId;
}

mixxx::OverviewType RenderTrackOverview::getType() const {
    return m_type;
}

WaveformSignalColors RenderTrackOverview::getSignalColors() const {
    return m_signalColors;
}

OverviewRenderThread::OverviewRenderThread(mixxx::DbConnectionPoolPtr dbConnectionPool,
        UserSettingsPointer pConfig)
        : WorkerThread("OverviewRenderThread"),
          m_dbConnectionPool(dbConnectionPool),
          m_pConfig(pConfig),
          m_nextTrack(2) {
}

bool OverviewRenderThread::scheduleRender(TrackId trackId,
        mixxx::OverviewType overviewType,
        WaveformSignalColors signalColors) {
    kLogger.info() << "scheduleRender" << trackId << overviewType;

    RenderTrackOverview rto(trackId, overviewType, signalColors);
    if (m_nextTrack.try_emplace(rto)) {
        // Ensure that the submitted track gets processed eventually
        // by waking the worker thread up after adding a new task to
        // its back queue! Otherwise the thread might not notice if
        // it is currently idle and has fallen asleep.
        wake();
        return true;
    }
    return false;
}

void OverviewRenderThread::doRun() {
    kLogger.info() << "doRun";

    mixxx::DbConnectionPooler dbConnectionPooler;
    dbConnectionPooler = mixxx::DbConnectionPooler(m_dbConnectionPool); // move assignment
    if (!dbConnectionPooler.isPooling()) {
        kLogger.warning() << "Failed to obtain database connection for overview render thread";
        return;
    }

    AnalysisDao analysisDao(m_pConfig);
    analysisDao.initialize(mixxx::DbConnectionPooled(m_dbConnectionPool));

    while (awaitWorkItemsFetched()) {
        TrackId trackId = m_currentTrack->getTrackId();
        mixxx::OverviewType type = m_currentTrack->getType();
        WaveformSignalColors signalColors = m_currentTrack->getSignalColors();

        kLogger.info() << "Rendering overview for track" << trackId;

        QList<AnalysisDao::AnalysisInfo> analyses =
                analysisDao.getAnalysesForTrackByType(
                        trackId, AnalysisDao::AnalysisType::TYPE_WAVESUMMARY);

        if (!analyses.isEmpty()) {
            ConstWaveformPointer pWaveform = ConstWaveformPointer(
                    WaveformFactory::loadWaveformFromAnalysis(
                            analyses.first()));

            RenderResult result = render(pWaveform, type, signalColors);

            // TODO: Save in cache.

            // Emit signal that overview waveform is rendered
            emit overviewRendered(trackId,
                    type,
                    signalColors,
                    result.image,
                    pWaveform->getCompletion());
        }
    }
}

WorkerThread::TryFetchWorkItemsResult OverviewRenderThread::tryFetchWorkItems() {
    DEBUG_ASSERT(!m_currentTrack.has_value());
    RenderTrackOverview* pFront = m_nextTrack.front();
    if (pFront) {
        m_currentTrack = *pFront;
        m_nextTrack.pop();
        kLogger.debug() << "Dequeued next track" << m_currentTrack->getTrackId();
        return TryFetchWorkItemsResult::Ready;
    } else {
        return TryFetchWorkItemsResult::Idle;
    }
}

// static
RenderResult OverviewRenderThread::render(ConstWaveformPointer pWaveform,
        mixxx::OverviewType type,
        WaveformSignalColors signalColors) {
    ScopedTimer t(QStringLiteral("WaveformOverviewRenderer::render"));

    const int dataSize = pWaveform->getDataSize();
    if (dataSize <= 0) {
        return RenderResult();
    }

    QImage image(dataSize / 2, 2 * 255, QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(0, 0, 0, 0).value());

    float peak = drawNextPixmapPart(image, pWaveform, type, signalColors, 0);

    RenderResult result;
    result.image = image;
    result.completion = pWaveform->getCompletion();
    result.waveformPeak = peak;
    return result;
}

// static
float OverviewRenderThread::drawNextPixmapPart(QImage& image,
        ConstWaveformPointer pWaveform,
        mixxx::OverviewType type,
        WaveformSignalColors signalColors,
        const int actualCompletion) {
    ScopedTimer t(QStringLiteral("WaveformOverviewRenderer::drawNextPixmapPart"));

    QPainter painter(&image);
    painter.translate(0.0, static_cast<double>(image.height()) / 2.0);

    const int nextCompletion = pWaveform->getCompletion();

    if (type == mixxx::OverviewType::Filtered) {
        drawNextPixmapPartLMH(&painter, pWaveform, signalColors, actualCompletion, nextCompletion);
    } else if (type == mixxx::OverviewType::HSV) {
        drawNextPixmapPartHSV(&painter, pWaveform, signalColors, actualCompletion, nextCompletion);
    } else { // mixxx::OverviewType::RGB:
        drawNextPixmapPartRGB(&painter, pWaveform, signalColors, actualCompletion, nextCompletion);
    }

    float peak = -1.0f;
    for (int i = 0; i < nextCompletion; i += 2) {
        peak = math_max3(peak,
                static_cast<float>(pWaveform->getAll(i)),
                static_cast<float>(pWaveform->getAll(i + 1)));
    }
    return peak;
}

void OverviewRenderThread::drawNextPixmapPartHSV(QPainter* pPainter,
        ConstWaveformPointer pWaveform,
        WaveformSignalColors signalColors,
        const int actualCompletion,
        const int nextCompletion) {
    ScopedTimer t(QStringLiteral("OverviewRenderThread::drawNextPixmapPartHSV"));

    // Get HSV of low color.
    float h, s, v;
    getHsvF(signalColors.getLowColor(), &h, &s, &v);

    QColor color;
    float lo, hi, total;

    unsigned char maxLow[2] = {0, 0};
    unsigned char maxHigh[2] = {0, 0};
    unsigned char maxMid[2] = {0, 0};
    unsigned char maxAll[2] = {0, 0};

    for (int currentCompletion = actualCompletion;
            currentCompletion < nextCompletion;
            currentCompletion += 2) {
        maxAll[0] = pWaveform->getAll(currentCompletion);
        maxAll[1] = pWaveform->getAll(currentCompletion + 1);
        if (maxAll[0] || maxAll[1]) {
            maxLow[0] = pWaveform->getLow(currentCompletion);
            maxLow[1] = pWaveform->getLow(currentCompletion + 1);
            maxMid[0] = pWaveform->getMid(currentCompletion);
            maxMid[1] = pWaveform->getMid(currentCompletion + 1);
            maxHigh[0] = pWaveform->getHigh(currentCompletion);
            maxHigh[1] = pWaveform->getHigh(currentCompletion + 1);

            total = (maxLow[0] + maxLow[1] + maxMid[0] + maxMid[1] +
                            maxHigh[0] + maxHigh[1]) *
                    1.2f;

            // Prevent division by zero
            if (total > 0) {
                // Normalize low and high
                // (mid not need, because it not change the color)
                lo = (maxLow[0] + maxLow[1]) / total;
                hi = (maxHigh[0] + maxHigh[1]) / total;
            } else {
                lo = hi = 0.0;
            }

            // Set color
            color.setHsvF(h, 1.0f - hi, 1.0f - lo);

            pPainter->setPen(color);
            pPainter->drawLine(QPoint(currentCompletion / 2, -maxAll[0]),
                    QPoint(currentCompletion / 2, maxAll[1]));
        }
    }
}

void OverviewRenderThread::drawNextPixmapPartLMH(QPainter* pPainter,
        ConstWaveformPointer pWaveform,
        WaveformSignalColors signalColors,
        const int actualCompletion,
        const int nextCompletion) {
    ScopedTimer t(QStringLiteral("OverviewRenderThread::drawNextPixmapPartLMH"));

    QColor lowColor = signalColors.getLowColor();
    QPen lowColorPen(QBrush(lowColor), 1);

    QColor midColor = signalColors.getMidColor();
    QPen midColorPen(QBrush(midColor), 1);

    QColor highColor = signalColors.getHighColor();
    QPen highColorPen(QBrush(highColor), 1);

    int currentCompletion = 0;
    for (currentCompletion = actualCompletion;
            currentCompletion < nextCompletion;
            currentCompletion += 2) {
        unsigned char lowNeg = pWaveform->getLow(currentCompletion);
        unsigned char lowPos = pWaveform->getLow(currentCompletion + 1);
        if (lowPos || lowNeg) {
            pPainter->setPen(lowColorPen);
            pPainter->drawLine(QPoint(currentCompletion / 2, -lowNeg),
                    QPoint(currentCompletion / 2, lowPos));
        }
    }

    for (currentCompletion = actualCompletion;
            currentCompletion < nextCompletion;
            currentCompletion += 2) {
        pPainter->setPen(midColorPen);
        pPainter->drawLine(QPoint(currentCompletion / 2,
                                   -pWaveform->getMid(currentCompletion)),
                QPoint(currentCompletion / 2,
                        pWaveform->getMid(currentCompletion + 1)));
    }

    for (currentCompletion = actualCompletion;
            currentCompletion < nextCompletion;
            currentCompletion += 2) {
        pPainter->setPen(highColorPen);
        pPainter->drawLine(QPoint(currentCompletion / 2,
                                   -pWaveform->getHigh(currentCompletion)),
                QPoint(currentCompletion / 2,
                        pWaveform->getHigh(currentCompletion + 1)));
    }
}

void OverviewRenderThread::drawNextPixmapPartRGB(QPainter* pPainter,
        ConstWaveformPointer pWaveform,
        WaveformSignalColors signalColors,
        const int actualCompletion,
        const int nextCompletion) {
    ScopedTimer t(QStringLiteral("OverviewRenderThread::drawNextPixmapPartRGB"));

    QColor color;

    float lowColor_r, lowColor_g, lowColor_b;
    getRgbF(signalColors.getRgbLowColor(), &lowColor_r, &lowColor_g, &lowColor_b);

    float midColor_r, midColor_g, midColor_b;
    getRgbF(signalColors.getRgbMidColor(), &midColor_r, &midColor_g, &midColor_b);

    float highColor_r, highColor_g, highColor_b;
    getRgbF(signalColors.getRgbHighColor(), &highColor_r, &highColor_g, &highColor_b);

    // int currentCompletion = 0;
    for (int currentCompletion = actualCompletion;
            currentCompletion < nextCompletion;
            currentCompletion += 2) {
        unsigned char left = pWaveform->getAll(currentCompletion);
        unsigned char right = pWaveform->getAll(currentCompletion + 1);

        // Retrieve "raw" LMH values from waveform
        float low = static_cast<float>(pWaveform->getLow(currentCompletion));
        float mid = static_cast<float>(pWaveform->getMid(currentCompletion));
        float high = static_cast<float>(pWaveform->getHigh(currentCompletion));

        // Do matrix multiplication
        float red = low * lowColor_r + mid * midColor_r + high * highColor_r;
        float green = low * lowColor_g + mid * midColor_g + high * highColor_g;
        float blue = low * lowColor_b + mid * midColor_b + high * highColor_b;

        // Normalize and draw
        float max = math_max3(red, green, blue);
        if (max > 0.0) {
            color.setRgbF(red / max, green / max, blue / max);
            pPainter->setPen(color);
            pPainter->drawLine(QPointF(currentCompletion / 2, -left),
                    QPointF(currentCompletion / 2, 0));
        }

        // Retrieve "raw" LMH values from waveform
        low = static_cast<float>(pWaveform->getLow(currentCompletion + 1));
        mid = static_cast<float>(pWaveform->getMid(currentCompletion + 1));
        high = static_cast<float>(pWaveform->getHigh(currentCompletion + 1));

        // Do matrix multiplication
        red = low * lowColor_r + mid * midColor_r + high * highColor_r;
        green = low * lowColor_g + mid * midColor_g + high * highColor_g;
        blue = low * lowColor_b + mid * midColor_b + high * highColor_b;

        // Normalize and draw
        max = math_max3(red, green, blue);
        if (max > 0.0) {
            color.setRgbF(red / max, green / max, blue / max);
            pPainter->setPen(color);
            pPainter->drawLine(QPointF(currentCompletion / 2, 0),
                    QPointF(currentCompletion / 2, right));
        }
    }
}
