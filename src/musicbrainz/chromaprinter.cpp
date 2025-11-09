#include "musicbrainz/chromaprinter.h"

#include <chromaprint.h>

#include <QtDebug>
#include <vector>

#include "moc_chromaprinter.cpp"
#include "sources/audiosourcestereoproxy.h"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "util/performancetimer.h"
#include "util/sample.h"

#define DEBUG_PRINT_FINGERPRINT

#ifdef DEBUG_PRINT_FINGERPRINT
#include <QCryptographicHash>
#endif

namespace
{

// Type declarations of *fprint and *encoded pointers need to account for Chromaprint API version
// (void* -> uint32_t*) and (void* -> char*) changed in versions v1.4.0 or later -- alyptik 12/2016
#if (CHROMAPRINT_VERSION_MINOR > 3) || (CHROMAPRINT_VERSION_MAJOR > 1)
    typedef uint32_t* uint32_p;
    typedef char* char_p;
#else
    typedef void* uint32_p;
    typedef void* char_p;
#endif

// this is worth 2min of audio
// AcoustID only stores a fingerprint for the first two minutes of a song
// on their server so we need only a fingerprint of the first two minutes
// --kain88 July 2012
    constexpr SINT kFingerprintDuration = 120; // in seconds

    std::vector<uint32_t> old;

#define MATCH_BITS 14
#define MATCH_MASK ((1 << MATCH_BITS) - 1)
#define MATCH_STRIP(x) ((uint32_t)(x) >> (32 - MATCH_BITS))

#define UNIQ_BITS 16
#define UNIQ_MASK ((1 << MATCH_BITS) - 1)
#define UNIQ_STRIP(x) ((uint32_t)(x) >> (32 - MATCH_BITS))

    int popcount_3(uint64_t x) {
        const uint64_t m1 = 0x5555555555555555ULL;  /* binary: 0101... */
        const uint64_t m2 = 0x3333333333333333ULL;  /* binary: 00110011.. */
        const uint64_t m4 = 0x0f0f0f0f0f0f0f0fULL;  /* binary:  4 zeros,  4 ones ... */
        const uint64_t h01 = 0x0101010101010101ULL; /* the sum of 256 to the power of 0,1,2,3... */

        x -= (x >> 1) & m1;             /* put count of each 2 bits into those 2 bits */
        x = (x & m2) + ((x >> 2) & m2); /* put count of each 4 bits into those 4 bits */
        x = (x + (x >> 4)) & m4;        /* put count of each 8 bits into those 8 bits */
        return (x * h01) >> 56;         /* returns left 8 bits of x + (x<<8) + (x<<16) +
                                           (x<<24) + ...  */
    }

    bool compareBit(uint32_t a, uint32_t b, int bit) {
        return ((a >> bit) & 1) == ((b >> bit) & 1);
    }

    float matchFingerprints(uint32_t* a, int aSize, uint32_t* b, int bSize, int maxoffset) {
        int biterror;
        int numcounts = aSize + bSize + 1;

        std::vector<unsigned short> counts(numcounts);

        float score;
        float diversity;

        uint16_t aOffsets[MATCH_MASK + 1] = {};
        uint16_t bOffsets[MATCH_MASK + 1] = {};

        // store offset of different patterns
        for (int i = 0; i < aSize; i++) {
            aOffsets[MATCH_STRIP(a[i])] = i;
        }
        for (int i = 0; i < bSize; i++) {
            bOffsets[MATCH_STRIP(b[i])] = i;
        }

        int topcount = 0;
        int topoffset = 0;
        // 16384 steps
        for (int i = 0; i < MATCH_MASK; i++) {
            if (aOffsets[i] && bOffsets[i]) {
                int offset = aOffsets[i] - bOffsets[i];
                if (maxoffset == 0 || (-maxoffset <= offset && offset <= maxoffset)) {
                    offset += bSize;
                    counts[offset]++;
                    if (counts[offset] > topcount) {
                        topcount = counts[offset];
                        topoffset = offset;
                    }
                }
            }
        }

        topoffset -= bSize;

        qDebug() << topoffset;

        int count_r1 = 0;
        int count_c1 = 0;
        int count_l1 = 0;
        int count_r2 = 0;
        int count_c2 = 0;
        int count_l2 = 0;

        int corr_size = std::min(aSize, bSize);

        if (topoffset > 0) {
            int i = 1 + topoffset;
            for (; i < (corr_size - (1 + topoffset)) / 2; i++) {
                for (int k = 0; k < 32; k++) {
                    if (compareBit(a[i - (-1 - topoffset)], b[i], k)) {
                        count_r1++;
                    }
                    if (compareBit(a[i - (0 - topoffset)], b[i], k)) {
                        count_c1++;
                    }
                    if (compareBit(a[i - (1 - topoffset)], b[i], k)) {
                        count_l1++;
                    }
                }
            }
            for (; i < corr_size - (1 + topoffset); i++) {
                for (int k = 0; k < 32; k++) {
                    if (compareBit(a[i - (-1 - topoffset)], b[i], k)) {
                        count_r2++;
                    }
                    if (compareBit(a[i - (0 - topoffset)], b[i], k)) {
                        count_c2++;
                    }
                    if (compareBit(a[i - (1 - topoffset)], b[i], k)) {
                        count_l2++;
                    }
                }
            }
        } else {
            int i = 1 - topoffset;
            for (; i < (corr_size - (1 - topoffset)) / 2; i++) {
                for (int k = 0; k < 32; k++) {
                    if (compareBit(a[i], b[i - (1 + topoffset)], k)) {
                        count_r1++;
                    }
                    if (compareBit(a[i], b[i - (0 + topoffset)], k)) {
                        count_c1++;
                    }
                    if (compareBit(a[i], b[i - (-1 + topoffset)], k)) {
                        count_l1++;
                    }
                }
            }
            for (; i < corr_size - (1 - topoffset); i++) {
                for (int k = 0; k < 32; k++) {
                    if (compareBit(a[i], b[i - (1 + topoffset)], k)) {
                        count_r2++;
                    }
                    if (compareBit(a[i], b[i - (0 + topoffset)], k)) {
                        count_c2++;
                    }
                    if (compareBit(a[i], b[i - (-1 + topoffset)], k)) {
                        count_l2++;
                    }
                }
            }
        }

        qDebug() << count_r1 << count_c1 << count_l1;

        int count_min1 = std::min(std::min(count_r1, count_c1), count_l1);

        count_r1 -= count_min1;
        count_c1 -= count_min1;
        count_l1 -= count_min1;

        float fract1 = ((float)count_r1 * 1 + float(count_l1) * -1) /
                (count_r1 + count_c1 + count_l1);

        qDebug() << count_r2 << count_c2 << count_l2;

        int count_min2 = std::min(std::min(count_r2, count_c2), count_l2);

        count_r2 -= count_min2;
        count_c2 -= count_min2;
        count_l2 -= count_min2;

        float fract2 = ((float)count_r2 * 1 + float(count_l2) * -1) /
                (count_r2 + count_c2 + count_l2);

        qDebug() << fract1 << fract2;

        const double A = 0.06;
        float correction = static_cast<float>(A * std::sin(2.0 * M_PI * (fract1 + fract2) / 2));
        qDebug() << "fine offset" << topoffset + (fract1 + fract2) / 2 - correction << correction;

        int minSize = std::min(aSize, bSize) & ~1;
        if (topoffset < 0) {
            b -= topoffset;
            bSize = std::max(0, bSize + topoffset);
        } else {
            a += topoffset;
            aSize = std::max(0, aSize - topoffset);
        }

        int size = std::min(aSize, bSize) / 2;
        if (!size || !minSize) {
            qDebug() << "acoustid_compare2: empty matching subfingerprint";
            return 0.0;
        }

        uint16_t seen[UNIQ_MASK + 1] = {};

        int aUniq = 0;
        for (int i = 0; i < aSize; i++) {
            int key = UNIQ_STRIP(a[i]);
            if (!seen[key]) {
                aUniq++;
                seen[key] = 1;
            }
        }

        memset(seen, 0, UNIQ_MASK);

        int bUniq = 0;
        for (int i = 0; i < bSize; i++) {
            int key = UNIQ_STRIP(b[i]);
            if (!seen[key]) {
                bUniq++;
                seen[key] = 1;
            }
        }

        diversity = std::min(std::min(1.0f, (float)(aUniq + 10) / aSize + 0.5f),
                std::min(1.0f, (float)(bUniq + 10) / bSize + 0.5f));

        qDebug() << "acoustid_compare2: offset" << topoffset << "offset score"
                 << topcount << "size" << size * 2 << "uniq size"
                 << std::max(aUniq, bUniq) << "diversity" << diversity;

        if (topcount < std::max(aUniq, bUniq) * 0.02) {
            qDebug() << "acoustid_compare2: top offset score is below 2% of the unique size";
            return 0.0;
        }

        uint64_t* aData = (uint64_t*)a;
        uint64_t* bData = (uint64_t*)b;
        biterror = 0;
        for (int i = 0; i < size; i++, aData++, bData++) {
            biterror += popcount_3(*aData ^ *bData);
        }
        score = (size * 2.0f / minSize) * (1.0f - 2.0f * (float)biterror / (64 * size));
        if (score < 0.0) {
            score = 0.0;
        }
        if (diversity < 1.0) {
            float newscore = std::pow(score, 8.0f - 7.0f * diversity);
            qDebug() << "acoustid_compare2: scaling score because of duplicate "
                        "items"
                     << score << "=>" << newscore;
            score = newscore;
        }
        return score;
    }

QString calcFingerprint(
        mixxx::AudioSourceStereoProxy& audioSourceProxy,
        mixxx::IndexRange fingerprintRange) {
    PerformanceTimer timerReadingFile;
    timerReadingFile.start();

    mixxx::SampleBuffer sampleBuffer(math_max(
            fingerprintRange.length(),
            audioSourceProxy.getSignalInfo().frames2samples(fingerprintRange.length())));
    const auto readableSampleFrames =
            audioSourceProxy.readSampleFrames(
                    mixxx::WritableSampleFrames(
                            fingerprintRange,
                            mixxx::SampleBuffer::WritableSlice(sampleBuffer)));
    if (fingerprintRange != readableSampleFrames.frameIndexRange()) {
        qWarning() << "Failed to read sample data for fingerprint";
        return QString();
    }

    std::vector<SAMPLE> fingerprintSamples(
            audioSourceProxy.getSignalInfo().frames2samples(
                    readableSampleFrames.frameLength()));

#ifdef DEBUG_PRINT_FINGERPRINT
    SampleUtil::applyGain(
            sampleBuffer.data(),
            1,
            sampleBuffer.size());
#endif

    // Convert floating-point to integer
    SampleUtil::convertFloat32ToS16(
            &fingerprintSamples[0],
            sampleBuffer.data(),
            static_cast<SINT>(fingerprintSamples.size()));

    qDebug() << "reading file took" << timerReadingFile.elapsed().debugMillisWithUnit();

    ChromaprintContext* ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);
    chromaprint_start(
            ctx,
            audioSourceProxy.getSignalInfo().getSampleRate(),
            audioSourceProxy.getSignalInfo().getChannelCount());

    PerformanceTimer timerGeneratingFingerprint;
    timerGeneratingFingerprint.start();
    const int success = chromaprint_feed(
            ctx,
            &fingerprintSamples[0],
            static_cast<int>(fingerprintSamples.size()));
    chromaprint_finish(ctx);
    if (!success) {
        qWarning() << "Failed to generate fingerprint from sample data";
        chromaprint_free(ctx);
        return QString();
    }

    uint32_p fprint = nullptr;
    int size = 0;
    int ret = chromaprint_get_raw_fingerprint(ctx, &fprint, &size);

#ifdef DEBUG_PRINT_FINGERPRINT
    uint32_t simHash;
    chromaprint_hash_fingerprint(fprint, size, &simHash);

    qDebug() << "similar"
             << matchFingerprints(fprint,
                        size,
                        old.data(),
                        static_cast<int>(old.size()),
                        120);

    QByteArray hash =
            QCryptographicHash::hash(QByteArray(reinterpret_cast<char*>(fprint),
                                             size * sizeof(fprint[0])),
                    QCryptographicHash::Md5)
                    .toHex();

    qDebug() << "Hash:" << hash;
    qDebug() << "SimHash:" << simHash;

    old.clear();
    for (int i = 0; i < size; ++i) {
        QString item = QString::number(fprint[i], 2).replace('0', ' ');
        while (item.length() < 32) {
            item.prepend(' ');
        }
        // qDebug() << item << i;
        old.push_back(fprint[i]);
    }
#endif

    QByteArray fingerprint;
    if (ret == 1) {
        char_p encoded = nullptr;
        int encoded_size = 0;
        int base64 = 1;
        chromaprint_encode_fingerprint(fprint,
                size,
                CHROMAPRINT_ALGORITHM_DEFAULT,
                &encoded,
                &encoded_size,
                base64);

        fingerprint.append(reinterpret_cast<char*>(encoded), encoded_size);

        chromaprint_dealloc(fprint);
        chromaprint_dealloc(encoded);
    }
    chromaprint_free(ctx);

    qDebug() << "generating fingerprint took"
             << timerGeneratingFingerprint.elapsed().debugMillisWithUnit();

    qDebug() << fingerprint;

    return fingerprint;
}

} // anonymous namespace

ChromaPrinter::ChromaPrinter(QObject* parent)
             : QObject(parent) {
}

QString ChromaPrinter::getFingerprint(TrackPointer pTrack) {
    mixxx::AudioSource::OpenParams config;
    // always stereo / 2 channels (see below)
    config.setChannelCount(mixxx::audio::ChannelCount(2));
    auto pAudioSource = SoundSourceProxy(pTrack).openAudioSource(config);
    if (!pAudioSource) {
        qDebug()
                << "Failed to open file for fingerprinting"
                << pTrack->getLocation();
        return QString();
    }

    const SINT startFrame = static_cast<SINT>(5944 * 2.25); // 5944 item size for 48 kHz Tracks
    mixxx::IndexRange fingerprintRange = intersect(
            pAudioSource->frameIndexRange(),
            mixxx::IndexRange::forward(
                    pAudioSource->frameIndexMin() + startFrame,
                    kFingerprintDuration * pAudioSource->getSignalInfo().getSampleRate()));
    mixxx::AudioSourceStereoProxy audioSourceProxy(
            pAudioSource,
            fingerprintRange.length());

    calcFingerprint(audioSourceProxy, fingerprintRange);

    fingerprintRange = intersect(
            pAudioSource->frameIndexRange(),
            mixxx::IndexRange::forward(
                    pAudioSource->frameIndexMin(),
                    kFingerprintDuration * pAudioSource->getSignalInfo().getSampleRate()));

    return calcFingerprint(audioSourceProxy, fingerprintRange);
}
