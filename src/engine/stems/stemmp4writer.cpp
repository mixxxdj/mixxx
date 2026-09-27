#include "stemmp4writer.h"

#include <QByteArray>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtEndian>

#include <algorithm>
#include <cmath>

#include "util/logger.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/frame.h>
#include <libavutil/samplefmt.h>
}

namespace mixxx {

namespace {

const mixxx::Logger kLogger("StemMp4Writer");

constexpr int kNumStems = 4;
constexpr int kNumStreams = 5; // main mix + 4 stems (native STEM layout)
constexpr int kAacBitRate = 256000;

// --- MP4 box helpers (big-endian) -------------------------------------------

quint32 readBE32(const QByteArray& data, qsizetype pos) {
    DEBUG_ASSERT(pos >= 0 && pos + 4 <= data.size());
    return (static_cast<quint32>(static_cast<quint8>(data[pos])) << 24) |
            (static_cast<quint32>(static_cast<quint8>(data[pos + 1])) << 16) |
            (static_cast<quint32>(static_cast<quint8>(data[pos + 2])) << 8) |
            static_cast<quint32>(static_cast<quint8>(data[pos + 3]));
}

void writeBE32(QByteArray* data, qsizetype pos, quint32 value) {
    DEBUG_ASSERT(data && pos >= 0 && pos + 4 <= data->size());
    (*data)[pos] = static_cast<char>((value >> 24) & 0xFF);
    (*data)[pos + 1] = static_cast<char>((value >> 16) & 0xFF);
    (*data)[pos + 2] = static_cast<char>((value >> 8) & 0xFF);
    (*data)[pos + 3] = static_cast<char>(value & 0xFF);
}

/// Returns the byte size of the box at `pos` including its header, honouring
/// the 64-bit extended size (size == 1). A size of 0 means "to end of file".
qint64 boxTotalSize(const QByteArray& data, qsizetype pos) {
    const quint32 size = readBE32(data, pos);
    if (size == 1) {
        // Extended size: 8-byte size follows the 8-byte header.
        quint64 extSize = 0;
        for (int i = 0; i < 8; ++i) {
            extSize = (extSize << 8) | static_cast<quint8>(data[pos + 8 + i]);
        }
        return static_cast<qint64>(extSize);
    }
    if (size == 0) {
        return data.size() - pos;
    }
    return size;
}

/// Locates the first top-level box of the given fourcc, returning the position
/// of its header, or -1 if not found.
qsizetype findTopLevelBox(const QByteArray& data, const char* fourcc) {
    qsizetype pos = 0;
    while (pos + 8 <= data.size()) {
        if (data.mid(pos + 4, 4) == QByteArray(fourcc, 4)) {
            return pos;
        }
        const qint64 size = boxTotalSize(data, pos);
        if (size < 8) {
            return -1;
        }
        pos += size;
    }
    return -1;
}

/// Locates a child box with the given fourcc inside the box whose payload
/// starts at `parentPayloadStart` (exclusive of the parent header) and spans
/// `parentPayloadSize` bytes. Returns the header position or -1.
qsizetype findChildBox(
        const QByteArray& data,
        qsizetype parentPayloadStart,
        qint64 parentPayloadSize,
        const char* fourcc) {
    qsizetype pos = parentPayloadStart;
    const qsizetype end = parentPayloadStart + parentPayloadSize;
    while (pos + 8 <= end) {
        if (data.mid(pos + 4, 4) == QByteArray(fourcc, 4)) {
            return pos;
        }
        const qint64 size = boxTotalSize(data, pos);
        if (size < 8) {
            return -1;
        }
        pos += size;
    }
    return -1;
}

QByteArray makeBoxHeader(quint32 payloadSize, const char fourcc[4]) {
    QByteArray box;
    box.reserve(8 + static_cast<int>(payloadSize));
    box.append(static_cast<char>((8 + payloadSize) >> 24));
    box.append(static_cast<char>((8 + payloadSize) >> 16 & 0xFF));
    box.append(static_cast<char>((8 + payloadSize) >> 8 & 0xFF));
    box.append(static_cast<char>((8 + payloadSize) & 0xFF));
    box.append(fourcc, 4);
    return box;
}

/// Inserts the `moov/udta/stem` box carrying the STEM manifest JSON into a
/// written MP4 file. Only the `moov` box (and `udta` inside it) sizes are
/// updated; since `moov` is the last box in libavformat's default layout,
/// inserting bytes at its end never invalidates sample offsets (stco/co64)
/// which point into `mdat`.
bool insertStemAtom(const QString& filePath, const StemMp4Writer::Config& config) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        kLogger.warning() << "Cannot open MP4 for stem atom insertion:" << filePath;
        return false;
    }
    QByteArray data = file.readAll();
    file.close();
    if (data.size() < 16) {
        kLogger.warning() << "MP4 file too small to contain a stem atom:" << filePath;
        return false;
    }

    // Build the STEM manifest JSON payload.
    QJsonArray stemsArray;
    for (int i = 0; i < kNumStems; ++i) {
        QJsonObject stemObj;
        stemObj.insert("name", config.stemNames[i]);
        stemObj.insert("color", config.stemColors[i]);
        stemsArray.append(stemObj);
    }
    QJsonObject manifest;
    manifest.insert("version", 1);
    manifest.insert("stems", stemsArray);
    const QByteArray jsonPayload = QJsonDocument(manifest).toJson(QJsonDocument::Compact);
    if (jsonPayload.isEmpty()) {
        kLogger.warning() << "Failed to build STEM manifest JSON";
        return false;
    }

    // stem box: [header]["stem"][json]
    QByteArray stemBox = makeBoxHeader(static_cast<quint32>(jsonPayload.size()), "stem");
    stemBox.append(jsonPayload);

    // Locate moov box.
    const qsizetype moovPos = findTopLevelBox(data, "moov");
    if (moovPos < 0) {
        kLogger.warning() << "No moov box found in MP4:" << filePath;
        return false;
    }
    const qint64 moovSize = boxTotalSize(data, moovPos);
    if (moovSize < 8) {
        return false;
    }
    const qsizetype moovPayloadStart = moovPos + 8;
    const qint64 moovPayloadSize = moovSize - 8;

    // Locate udta box inside moov.
    const qsizetype udtaPos = findChildBox(data, moovPayloadStart, moovPayloadSize, "udta");
    if (udtaPos < 0) {
        // No udta box: append one (holding the stem box) at the end of moov.
        QByteArray udtaBox = makeBoxHeader(
                static_cast<quint32>(stemBox.size()), "udta");
        udtaBox.append(stemBox);
        const qsizetype insertPos = moovPos + moovSize;
        data.insert(insertPos, udtaBox);
        // Grow moov size to cover the new udta box.
        writeBE32(&data, moovPos, static_cast<quint32>(moovSize + udtaBox.size()));
    } else {
        // Insert the stem box at the end of the existing udta box and grow the
        // udta and moov sizes accordingly.
        const qint64 udtaSize = boxTotalSize(data, udtaPos);
        const qsizetype insertPos = udtaPos + udtaSize;
        data.insert(insertPos, stemBox);
        writeBE32(&data, udtaPos, static_cast<quint32>(udtaSize + stemBox.size()));
        writeBE32(&data, moovPos, static_cast<quint32>(moovSize + stemBox.size()));
    }

    // Write back (moov is the last box, so the file just grows at the end).
    if (!file.open(QIODevice::WriteOnly)) {
        kLogger.warning() << "Cannot open MP4 for writing stem atom:" << filePath;
        return false;
    }
    file.write(data);
    file.close();
    kLogger.debug() << "Inserted stem atom (" << jsonPayload.size()
                    << " byte manifest) into:" << filePath;
    return true;
}

// --- libavformat AAC encoding -------------------------------------------------

/// Encodes `config.stems` into a plain 5-stream AAC MP4 at `tmpPath`.
/// Stream 0 (main mix) is the clamped sum of the 4 stems; streams 1..4 are
/// the individual stems (a null buffer encodes as silence).
bool writeMp4(const QString& tmpPath, const StemMp4Writer::Config& config) {
    AVFormatContext* fmtCtx = nullptr;
    const int ret = avformat_alloc_output_context2(
            &fmtCtx, nullptr, "mp4", tmpPath.toUtf8().constData());
    if (ret < 0 || !fmtCtx) {
        kLogger.warning() << "avformat_alloc_output_context2 failed:" << ret;
        return false;
    }

    const AVCodec* pCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!pCodec) {
        kLogger.warning() << "AAC encoder not available";
        avformat_free_context(fmtCtx);
        return false;
    }

    AVStream* streams[kNumStreams] = {};
    AVCodecContext* codecCtxs[kNumStreams] = {};

    for (int s = 0; s < kNumStreams; ++s) {
        AVStream* pStream = avformat_new_stream(fmtCtx, pCodec);
        AVCodecContext* pCodecCtx = avcodec_alloc_context3(pCodec);
        if (!pStream || !pCodecCtx) {
            kLogger.warning() << "Failed to allocate stream" << s;
            for (int i = 0; i < s; ++i) {
                avcodec_free_context(&codecCtxs[i]);
            }
            avformat_free_context(fmtCtx);
            return false;
        }
        streams[s] = pStream;
        codecCtxs[s] = pCodecCtx;

        pCodecCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;
        pCodecCtx->sample_rate = config.sampleRate;
        pCodecCtx->bit_rate = kAacBitRate;
        pCodecCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
        av_channel_layout_default(&pCodecCtx->ch_layout, 2); // stereo

        if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0) {
            kLogger.warning() << "avcodec_open2 failed for stream" << s;
            for (int i = 0; i <= s; ++i) {
                avcodec_free_context(&codecCtxs[i]);
            }
            avformat_free_context(fmtCtx);
            return false;
        }

        pStream->time_base = AVRational{1, config.sampleRate};
        pStream->id = s;
        if (avcodec_parameters_from_context(pStream->codecpar, pCodecCtx) < 0) {
            kLogger.warning() << "avcodec_parameters_from_context failed for stream" << s;
            for (int i = 0; i <= s; ++i) {
                avcodec_free_context(&codecCtxs[i]);
            }
            avformat_free_context(fmtCtx);
            return false;
        }
    }

    if (avio_open(&fmtCtx->pb, tmpPath.toUtf8().constData(), AVIO_FLAG_WRITE) < 0) {
        kLogger.warning() << "avio_open failed:" << tmpPath;
        for (int i = 0; i < kNumStreams; ++i) {
            avcodec_free_context(&codecCtxs[i]);
        }
        avformat_free_context(fmtCtx);
        return false;
    }

    if (avformat_write_header(fmtCtx, nullptr) < 0) {
        kLogger.warning() << "avformat_write_header failed";
        avio_closep(&fmtCtx->pb);
        for (int i = 0; i < kNumStreams; ++i) {
            avcodec_free_context(&codecCtxs[i]);
        }
        avformat_free_context(fmtCtx);
        return false;
    }

    // Pre-compute the main mix (clamped sum of the stems). A null stem buffer
    // contributes silence.
    QVector<float> mainMix;
    if (config.numFrames > 0) {
        mainMix.resize(static_cast<int>(config.numFrames) * 2);
        for (qsizetype i = 0; i < mainMix.size(); ++i) {
            float sum = 0.0f;
            for (int s = 0; s < kNumStems; ++s) {
                if (config.stems[s]) {
                    sum += config.stems[s][i];
                }
            }
            mainMix[i] = std::clamp(sum, -1.0f, 1.0f);
        }
    }
    const float* streamSource[kNumStreams] = {
            mainMix.constData(),
            config.stems[0],
            config.stems[1],
            config.stems[2],
            config.stems[3],
    };

    const int frameSize = codecCtxs[0]->frame_size > 0
            ? codecCtxs[0]->frame_size
            : 1024; // AAC default frame size

    AVFrame* pFrame = av_frame_alloc();
    AVPacket* pPkt = av_packet_alloc();
    if (!pFrame || !pPkt) {
        kLogger.warning() << "Failed to allocate frame/packet";
        av_packet_free(&pPkt);
        av_frame_free(&pFrame);
        av_write_trailer(fmtCtx);
        avio_closep(&fmtCtx->pb);
        for (int i = 0; i < kNumStreams; ++i) {
            avcodec_free_context(&codecCtxs[i]);
        }
        avformat_free_context(fmtCtx);
        return false;
    }
    pFrame->format = AV_SAMPLE_FMT_FLTP;
    pFrame->sample_rate = config.sampleRate;
    av_channel_layout_default(&pFrame->ch_layout, 2);

    bool ok = true;
    qint64 framesRemaining = config.numFrames;
    int frameIdx = 0;
    while (ok && framesRemaining > 0) {
        const int n = static_cast<int>(std::min<qint64>(frameSize, framesRemaining));
        // av_frame_unref() at the end of the loop wipes format/ch_layout,
        // so re-initialize them every iteration before allocating buffers.
        av_frame_unref(pFrame);
        pFrame->format = AV_SAMPLE_FMT_FLTP;
        pFrame->sample_rate = config.sampleRate;
        av_channel_layout_uninit(&pFrame->ch_layout);
        av_channel_layout_default(&pFrame->ch_layout, 2);
        pFrame->nb_samples = n;
        pFrame->pts = static_cast<int64_t>(frameIdx) * frameSize; // in codec time_base
        if (av_frame_get_buffer(pFrame, 0) < 0) {
            ok = false;
            break;
        }
        if (av_frame_make_writable(pFrame) < 0) {
            ok = false;
            av_frame_unref(pFrame);
            break;
        }

        for (int s = 0; s < kNumStreams && ok; ++s) {
            const float* src = streamSource[s];
            // Interleaved (L,R,L,R,...) -> planar (L... / R...)
            float* dstL = reinterpret_cast<float*>(pFrame->data[0]);
            float* dstR = reinterpret_cast<float*>(pFrame->data[1]);
            const qsizetype base = static_cast<qsizetype>(frameIdx) * frameSize * 2;
            if (src) {
                for (int i = 0; i < n; ++i) {
                    dstL[i] = src[base + i * 2];
                    dstR[i] = src[base + i * 2 + 1];
                }
            } else {
                std::fill_n(dstL, n, 0.0f);
                std::fill_n(dstR, n, 0.0f);
            }

            if (avcodec_send_frame(codecCtxs[s], pFrame) < 0) {
                ok = false;
                break;
            }
            while (true) {
                const int recvRet = avcodec_receive_packet(codecCtxs[s], pPkt);
                if (recvRet == AVERROR(EAGAIN) || recvRet == AVERROR_EOF) {
                    break;
                }
                if (recvRet < 0) {
                    ok = false;
                    break;
                }
                av_packet_rescale_ts(pPkt, codecCtxs[s]->time_base, streams[s]->time_base);
                pPkt->stream_index = streams[s]->index;
                if (av_interleaved_write_frame(fmtCtx, pPkt) < 0) {
                    ok = false;
                    av_packet_unref(pPkt);
                    break;
                }
                av_packet_unref(pPkt);
            }
        }
        av_frame_unref(pFrame);
        framesRemaining -= n;
        ++frameIdx;
    }

    // Flush remaining encoder frames.
    for (int s = 0; s < kNumStreams && ok; ++s) {
        avcodec_send_frame(codecCtxs[s], nullptr);
        while (true) {
            const int recvRet = avcodec_receive_packet(codecCtxs[s], pPkt);
            if (recvRet == AVERROR(EAGAIN) || recvRet == AVERROR_EOF) {
                break;
            }
            if (recvRet < 0) {
                ok = false;
                break;
            }
            av_packet_rescale_ts(pPkt, codecCtxs[s]->time_base, streams[s]->time_base);
            pPkt->stream_index = streams[s]->index;
            if (av_interleaved_write_frame(fmtCtx, pPkt) < 0) {
                ok = false;
                av_packet_unref(pPkt);
                break;
            }
            av_packet_unref(pPkt);
        }
    }

    av_write_trailer(fmtCtx);
    av_packet_free(&pPkt);
    av_frame_free(&pFrame);
    avio_closep(&fmtCtx->pb);
    for (int i = 0; i < kNumStreams; ++i) {
        avcodec_free_context(&codecCtxs[i]);
    }
    avformat_free_context(fmtCtx);

    if (!ok) {
        kLogger.warning() << "AAC encoding failed";
    }
    return ok;
}

} // anonymous namespace

bool StemMp4Writer::write(const Config& config) {
    if (config.outputPath.isEmpty()) {
        kLogger.warning() << "Empty output path";
        return false;
    }
    if (config.numFrames <= 0) {
        kLogger.warning() << "No frames to write";
        return false;
    }

    // Write atomically: temp file + rename, so a failed/cancelled run never
    // leaves a half-written .stem.mp4 behind.
    const QString tmpPath = config.outputPath + QStringLiteral(".tmp");
    QFile::remove(tmpPath);

    if (!writeMp4(tmpPath, config)) {
        QFile::remove(tmpPath);
        return false;
    }
    if (!insertStemAtom(tmpPath, config)) {
        QFile::remove(tmpPath);
        return false;
    }

    QFile::remove(config.outputPath);
    if (!QFile::rename(tmpPath, config.outputPath)) {
        kLogger.warning() << "Failed to rename temp stem file to" << config.outputPath;
        QFile::remove(tmpPath);
        return false;
    }

    kLogger.info() << "Wrote native stem file:" << config.outputPath
                   << "(" << config.numFrames << "frames @"
                   << config.sampleRate << "Hz)";
    return true;
}

} // namespace mixxx
