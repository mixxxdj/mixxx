#include "library/youtube/youtubeaudiocutter.h"

#include <QFile>
#include <QFileInfo>
#include <algorithm>

#include "library/youtube/youtubeservice.h" // SponsorSegment
#include "util/logger.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/mathematics.h>
}

namespace mixxx {

namespace {
const Logger kLogger("YouTubeAudioCutter");

struct Range {
    double start;
    double end;
};

/// Returns the cut ranges in chronological, non-overlapping form, plus the
/// total cut duration. Ranges from SponsorBlock can overlap (e.g. an "intro"
/// and a "selfpromo" segment both covering 0:00–0:15) and we want to merge
/// them so the running offset we apply to PTS is monotonic.
QList<Range> normaliseRanges(const QList<SponsorSegment>& segments, double& totalCut) {
    QList<Range> ranges;
    ranges.reserve(segments.size());
    for (const auto& s : segments) {
        if (s.end > s.start) {
            ranges.append({s.start, s.end});
        }
    }
    std::sort(ranges.begin(), ranges.end(), [](const Range& a, const Range& b) {
        return a.start < b.start;
    });
    QList<Range> merged;
    for (const auto& r : ranges) {
        if (!merged.isEmpty() && r.start <= merged.last().end) {
            merged.last().end = std::max(merged.last().end, r.end);
        } else {
            merged.append(r);
        }
    }
    totalCut = 0.0;
    for (const auto& r : merged) {
        totalCut += r.end - r.start;
    }
    return merged;
}

/// Returns the accumulated cut time strictly *before* `tsSec`, used to shift
/// post-cut PTS back to a contiguous timeline.
double cutTimeBefore(const QList<Range>& ranges, double tsSec) {
    double acc = 0.0;
    for (const auto& r : ranges) {
        if (tsSec >= r.end) {
            acc += r.end - r.start;
        } else {
            break;
        }
    }
    return acc;
}

bool inAnyRange(const QList<Range>& ranges, double tsSec) {
    for (const auto& r : ranges) {
        if (tsSec >= r.start && tsSec < r.end) {
            return true;
        }
        if (tsSec < r.start) {
            return false; // sorted, can't be in a later range
        }
    }
    return false;
}

} // namespace

bool cutAudioRanges(const QString& path, const QList<SponsorSegment>& segments) {
    if (segments.isEmpty()) {
        return true; // nothing to do, treat as success
    }
    double totalCut = 0.0;
    const QList<Range> ranges = normaliseRanges(segments, totalCut);
    if (ranges.isEmpty() || totalCut <= 0.0) {
        return true;
    }

    // Keep the input's extension on the temp file: FFmpeg guesses the output
    // container from the filename when format_name is null, and ".tmp"
    // matches nothing (avformat_alloc_output_context2 returns null, every
    // cut fails). ".cut.<ext>" preserves the container across the rename.
    const QString suffix = path.mid(path.lastIndexOf(QLatin1Char('.')));
    const QString tmpPath = path + QStringLiteral(".cut") + suffix;
    const QByteArray inPath = path.toUtf8();
    const QByteArray outPath = tmpPath.toUtf8();

    AVFormatContext* ic = nullptr;
    if (avformat_open_input(&ic, inPath.constData(), nullptr, nullptr) < 0) {
        kLogger.warning() << "cutAudioRanges: cannot open input" << path;
        return false;
    }
    if (avformat_find_stream_info(ic, nullptr) < 0) {
        avformat_close_input(&ic);
        kLogger.warning() << "cutAudioRanges: no stream info for" << path;
        return false;
    }

    AVFormatContext* oc = nullptr;
    if (avformat_alloc_output_context2(&oc, nullptr, nullptr, outPath.constData()) < 0 || !oc) {
        avformat_close_input(&ic);
        kLogger.warning() << "cutAudioRanges: cannot allocate output for" << tmpPath;
        return false;
    }

    // Stream-copy mapping: clone codecpar from each input stream into a new
    // output stream with the same time_base. We cut at packet boundaries so
    // no re-encoding is required.
    QList<int> streamMap(ic->nb_streams, -1);
    for (unsigned i = 0; i < ic->nb_streams; ++i) {
        AVStream* is = ic->streams[i];
        // Skip non-audio streams (YouTube adaptive audio downloads are
        // single-stream, but be defensive).
        if (is->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
            continue;
        }
        AVStream* os = avformat_new_stream(oc, nullptr);
        if (!os || avcodec_parameters_copy(os->codecpar, is->codecpar) < 0) {
            avformat_close_input(&ic);
            avformat_free_context(oc);
            kLogger.warning() << "cutAudioRanges: stream copy setup failed";
            return false;
        }
        os->time_base = is->time_base;
        os->codecpar->codec_tag = 0;
        streamMap[i] = os->index;
    }

    if (!(oc->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&oc->pb, outPath.constData(), AVIO_FLAG_WRITE) < 0) {
            avformat_close_input(&ic);
            avformat_free_context(oc);
            kLogger.warning() << "cutAudioRanges: avio_open failed for" << tmpPath;
            return false;
        }
    }

    if (avformat_write_header(oc, nullptr) < 0) {
        avio_closep(&oc->pb);
        avformat_close_input(&ic);
        avformat_free_context(oc);
        kLogger.warning() << "cutAudioRanges: write_header failed";
        return false;
    }

    AVPacket* pkt = av_packet_alloc();
    if (!pkt) {
        avio_closep(&oc->pb);
        avformat_close_input(&ic);
        avformat_free_context(oc);
        return false;
    }

    bool ok = true;
    while (av_read_frame(ic, pkt) >= 0) {
        const int oi = streamMap.value(pkt->stream_index, -1);
        if (oi < 0) {
            av_packet_unref(pkt);
            continue;
        }
        AVStream* is = ic->streams[pkt->stream_index];
        AVStream* os = oc->streams[oi];
        const double tsSec = pkt->pts == AV_NOPTS_VALUE
                ? 0.0
                : pkt->pts * av_q2d(is->time_base);
        if (inAnyRange(ranges, tsSec)) {
            av_packet_unref(pkt);
            continue;
        }
        // Shift PTS/DTS back by the total cut time accumulated before this
        // packet, so the output has a contiguous timeline starting at 0.
        const double shiftSec = cutTimeBefore(ranges, tsSec);
        const int64_t shiftTicks = static_cast<int64_t>(
                shiftSec / av_q2d(is->time_base));
        if (pkt->pts != AV_NOPTS_VALUE) {
            pkt->pts = av_rescale_q(pkt->pts - shiftTicks, is->time_base, os->time_base);
        }
        if (pkt->dts != AV_NOPTS_VALUE) {
            pkt->dts = av_rescale_q(pkt->dts - shiftTicks, is->time_base, os->time_base);
        }
        pkt->duration = av_rescale_q(pkt->duration, is->time_base, os->time_base);
        pkt->stream_index = oi;
        pkt->pos = -1;
        if (av_interleaved_write_frame(oc, pkt) < 0) {
            ok = false;
            av_packet_unref(pkt);
            break;
        }
        av_packet_unref(pkt);
    }

    av_write_trailer(oc);
    av_packet_free(&pkt);
    avio_closep(&oc->pb);
    avformat_close_input(&ic);
    avformat_free_context(oc);

    if (!ok) {
        QFile::remove(tmpPath);
        return false;
    }

    // Atomic-ish replace: remove old, rename new. On Android both files live
    // in the app data dir so this is a single-filesystem rename.
    if (!QFile::remove(path)) {
        QFile::remove(tmpPath);
        kLogger.warning() << "cutAudioRanges: cannot remove original" << path;
        return false;
    }
    if (!QFile::rename(tmpPath, path)) {
        kLogger.warning() << "cutAudioRanges: rename failed" << tmpPath << "->" << path;
        return false;
    }
    kLogger.info() << "Removed" << QString::number(totalCut, 'f', 1)
                   << "s of sponsor/intro/etc segments from" << QFileInfo(path).fileName();
    return true;
}

} // namespace mixxx
