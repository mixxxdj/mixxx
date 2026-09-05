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

int paintedPixelCount(const QImage& image) {
    int count = 0;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (qAlpha(image.pixel(x, y)) != 0) {
                ++count;
            }
        }
    }
    return count;
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

void setWaveformSample(Waveform* pWaveform, int index) {
    WaveformData* pData = pWaveform->data();
    pData[index].filtered.low = 32;
    pData[index].filtered.mid = 96;
    pData[index].filtered.high = 160;
    pData[index].filtered.all = 192;
}

class QmlWaveformOverviewTest : public MixxxTest {};

TEST_F(QmlWaveformOverviewTest, ProgressiveCacheAppendsAndResets) {
    TrackPointer pTrack = Track::newTemporary();
    WaveformPointer pWaveform(new Waveform(44100, 44100, 1000, 8, 0));
    ASSERT_GE(pWaveform->getDataSize(), 4);
    pTrack->setWaveformSummary(pWaveform);

    QmlTrackProxy trackProxy(pTrack);
    QmlWaveformOverview overview;
    overview.setWidth(100);
    overview.setHeight(24);
    overview.setTrack(&trackProxy);

    EXPECT_EQ(0, paintedPixelCount(paintOverview(&overview)));

    setWaveformSample(pWaveform.get(), 0);
    setWaveformSample(pWaveform.get(), 1);
    pWaveform->setCompletion(2);
    overview.setAnalyzerProgress(0.25);
    const int firstProgressPixelCount = paintedPixelCount(paintOverview(&overview));
    EXPECT_GT(firstProgressPixelCount, 0);

    setWaveformSample(pWaveform.get(), 2);
    setWaveformSample(pWaveform.get(), 3);
    pWaveform->setCompletion(4);
    overview.setAnalyzerProgress(0.5);
    EXPECT_GT(paintedPixelCount(paintOverview(&overview)), firstProgressPixelCount);

    pWaveform->setCompletion(0);
    overview.setAnalyzerProgress(0.0);
    EXPECT_EQ(0, paintedPixelCount(paintOverview(&overview)));
}

} // namespace
