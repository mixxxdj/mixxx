#pragma once

#include <QCache>

#include "control/controlproxy.h"
#include "rigtorp/SPSCQueue.h"
#include "util/db/dbconnectionpool.h"
#include "util/singleton.h"
#include "util/workerthread.h"
#include "waveform/overviews/overviewtype.h"
#include "waveform/renderers/waveformsignalcolors.h"
#include "waveform/waveform.h"
#include "widget/woverview.h"

class RenderTrackOverview {
  public:
    explicit RenderTrackOverview(TrackId trackId,
            mixxx::OverviewType type,
            WaveformSignalColors signalColors);

    TrackId getTrackId() const;

    mixxx::OverviewType getType() const;

    WaveformSignalColors getSignalColors() const;

  private:
    TrackId m_trackId;
    mixxx::OverviewType m_type;
    WaveformSignalColors m_signalColors;
};

struct RenderResult {
    QImage image;
    int completion;
    float waveformPeak;
};

class OverviewRenderThread : public WorkerThread {
    Q_OBJECT

  public:
    OverviewRenderThread(mixxx::DbConnectionPoolPtr dbConnectionPool, UserSettingsPointer pConfig);
    ~OverviewRenderThread() override = default;

    bool scheduleRender(TrackId, mixxx::OverviewType, WaveformSignalColors);

    static RenderResult render(ConstWaveformPointer, mixxx::OverviewType, WaveformSignalColors);

    static float drawNextPixmapPart(QImage& image,
            ConstWaveformPointer pWaveform,
            mixxx::OverviewType type,
            WaveformSignalColors signalColors,
            const int actualCompletion);

  signals:
    void overviewRendered(TrackId,
            mixxx::OverviewType,
            WaveformSignalColors,
            QImage,
            int completion);

  protected:
    void doRun() override;

    TryFetchWorkItemsResult tryFetchWorkItems() override;

  private:
    static void drawNextPixmapPartHSV(QPainter* pPainter,
            ConstWaveformPointer pWaveform,
            WaveformSignalColors signalColors,
            const int actualCompletion,
            const int nextCompletion);
    static void drawNextPixmapPartLMH(QPainter* pPainter,
            ConstWaveformPointer pWaveform,
            WaveformSignalColors signalColors,
            const int actualCompletion,
            const int nextCompletion);
    static void drawNextPixmapPartRGB(QPainter* pPainter,
            ConstWaveformPointer pWaveform,
            WaveformSignalColors signalColors,
            const int actualCompletion,
            const int nextCompletion);

    const mixxx::DbConnectionPoolPtr m_dbConnectionPool;
    const UserSettingsPointer m_pConfig;

    rigtorp::SPSCQueue<RenderTrackOverview> m_nextTrack;

    std::optional<RenderTrackOverview> m_currentTrack;
};
