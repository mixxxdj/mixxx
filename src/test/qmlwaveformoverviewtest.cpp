#include <gtest/gtest.h>

#include <QImage>
#include <QPainter>
#include <QtGlobal>

#include "qml/qmltrackproxy.h"
#include "qml/qmlwaveformoverview.h"
#include "test/mixxxtest.h"
#include "track/track.h"
#include "waveform/waveform.h"

using namespace mixxx::qml;

namespace {

bool hasPaintedPixels(const QImage& image) {
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (qAlpha(image.pixel(x, y)) != 0) {
                return true;
            }
        }
    }
    return false;
}

QImage paintOverview(QmlWaveformOverview* pOverview) {
    QImage image(qRound(pOverview->width()),
            qRound(pOverview->height()),
            QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    pOverview->paint(&painter);
    return image;
}

bool imagesDiffer(const QImage& lhs, const QImage& rhs) {
    return !(lhs == rhs);
}

void setWaveformSample(Waveform* pWaveform, int index) {
    WaveformData* pData = pWaveform->data();
    pData[index].filtered.low = 32;
    pData[index].filtered.mid = 96;
    pData[index].filtered.high = 160;
    pData[index].filtered.all = 192;
}

class QmlWaveformOverviewTest : public MixxxTest {};

TEST_F(QmlWaveformOverviewTest, RendersAvailableSummaryWithoutAnalyzerProgress) {
    TrackPointer pTrack = Track::newTemporary();
    WaveformPointer pWaveform(new Waveform(44100, 44100, 1000, 8, 0));
    ASSERT_GE(pWaveform->getDataSize(), 4);
    pTrack->setWaveformSummary(pWaveform);

    QmlTrackProxy trackProxy(pTrack);
    QmlWaveformOverview overview;
    overview.setWidth(100);
    overview.setHeight(24);
    overview.setTrack(&trackProxy);

    const QImage empty = paintOverview(&overview);
    EXPECT_FALSE(hasPaintedPixels(empty));

    setWaveformSample(pWaveform.get(), 0);
    setWaveformSample(pWaveform.get(), 1);
    pWaveform->setCompletion(2);
    const QImage partial = paintOverview(&overview);
    ASSERT_TRUE(hasPaintedPixels(partial));
    EXPECT_TRUE(imagesDiffer(empty, partial));

    setWaveformSample(pWaveform.get(), 2);
    setWaveformSample(pWaveform.get(), 3);
    pWaveform->setCompletion(4);
    const QImage complete = paintOverview(&overview);
    ASSERT_TRUE(hasPaintedPixels(complete));
    EXPECT_TRUE(imagesDiffer(partial, complete));

    overview.setRenderer(QmlWaveformOverview::Renderer::Filtered);
    const QImage filtered = paintOverview(&overview);
    ASSERT_TRUE(hasPaintedPixels(filtered));
    EXPECT_TRUE(imagesDiffer(complete, filtered));

    overview.setRenderer(QmlWaveformOverview::Renderer::HSV);
    const QImage hsv = paintOverview(&overview);
    ASSERT_TRUE(hasPaintedPixels(hsv));
    EXPECT_TRUE(imagesDiffer(filtered, hsv));

    overview.setStereo(false);
    const QImage mono = paintOverview(&overview);
    ASSERT_TRUE(hasPaintedPixels(mono));
    EXPECT_TRUE(imagesDiffer(hsv, mono));

    overview.setNormalized(true);
    const QImage normalized = paintOverview(&overview);
    ASSERT_TRUE(hasPaintedPixels(normalized));
    EXPECT_TRUE(imagesDiffer(mono, normalized));
}

TEST_F(QmlWaveformOverviewTest, MonoRgbColorUsesBothChannels) {
    TrackPointer pTrack = Track::newTemporary();
    WaveformPointer pWaveform(new Waveform(44100, 44100, 1000, 8, 0));
    ASSERT_GE(pWaveform->getDataSize(), 2);
    pWaveform->data()[0].filtered = {0, 0, 255, 255};
    pWaveform->data()[1].filtered = {255, 0, 0, 255};
    pWaveform->setCompletion(2);
    pTrack->setWaveformSummary(pWaveform);

    QmlTrackProxy trackProxy(pTrack);
    QmlWaveformOverview overview;
    overview.setWidth(16);
    overview.setHeight(24);
    overview.setStereo(false);
    overview.setTrack(&trackProxy);

    const QImage image = paintOverview(&overview);
    bool hasCombinedColor = false;
    for (int y = 0; y < image.height() && !hasCombinedColor; ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const QColor color = image.pixelColor(x, y);
            if (color.alpha() > 0 && color.red() > 200 && color.blue() > 200 &&
                    color.green() < 80) {
                hasCombinedColor = true;
                break;
            }
        }
    }
    EXPECT_TRUE(hasCombinedColor);
}

} // namespace
