#include "musicbrainz/chromaprinter.h"

#include <chromaprint.h>
#include <vector>

#include <QtDebug>

#include "sources/soundsourceproxy.h"
#include "sources/audiosourcestereoproxy.h"
#include "util/sample.h"
#include "util/performancetimer.h"

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
const SINT kFingerprintDuration = 120; // in seconds

QString calcFingerprint(
        mixxx::AudioSourceStereoProxy& audioSourceProxy,
        mixxx::IndexRange fingerprintRange) {
    PerformanceTimer timerReadingFile;
    timerReadingFile.start();

    mixxx::SampleBuffer sampleBuffer(math_max(
            fingerprintRange.length(),
            audioSourceProxy.frames2samples(fingerprintRange.length())));
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
            audioSourceProxy.frames2samples(
                    readableSampleFrames.frameLength()));
    // Convert floating-point to integer
    SampleUtil::convertFloat32ToS16(
            &fingerprintSamples[0],
            sampleBuffer.data(),
            fingerprintSamples.size());

    qDebug() << "reading file took" << timerReadingFile.elapsed().debugMillisWithUnit();

    ChromaprintContext* ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);
    chromaprint_start(ctx, audioSourceProxy.sampleRate(), audioSourceProxy.channelCount());

    PerformanceTimer timerGeneratingFingerprint;
    timerGeneratingFingerprint.start();
    int success = chromaprint_feed(ctx, &fingerprintSamples[0], fingerprintSamples.size());
    chromaprint_finish(ctx);
    if (!success) {
        qWarning() << "Failed to generate fingerprint from sample data";
        chromaprint_free(ctx);
        return QString();
    }

    uint32_p fprint = NULL;
    int size = 0;
    int ret = chromaprint_get_raw_fingerprint(ctx, &fprint, &size);
    QByteArray fingerprint;
    if (ret == 1) {
        char_p encoded = NULL;
        int encoded_size = 0;
        chromaprint_encode_fingerprint(fprint, size,
                                       CHROMAPRINT_ALGORITHM_DEFAULT,
                                       &encoded,
                                       &encoded_size, 1);

        fingerprint.append(reinterpret_cast<char*>(encoded), encoded_size);

        chromaprint_dealloc(fprint);
        chromaprint_dealloc(encoded);
    }
    chromaprint_free(ctx);

    qDebug() << "generating fingerprint took"
             << timerGeneratingFingerprint.elapsed().debugMillisWithUnit();

    return fingerprint;
}

} // anonymous namespace

ChromaPrinter::ChromaPrinter(QObject* parent)
             : QObject(parent) {
}

QString ChromaPrinter::getFingerprint(TrackPointer pTrack) {
    mixxx::AudioSource::OpenParams config;
    config.setChannelCount(2); // always stereo / 2 channels (see below)
    auto pAudioSource = SoundSourceProxy(pTrack).openAudioSource(config);
    if (!pAudioSource) {
        qDebug()
                << "Failed to open file for fingerprinting"
                << pTrack->getLocation();
        return QString();
    }

    const auto fingerprintRange = intersect(
            pAudioSource->frameIndexRange(),
            mixxx::IndexRange::forward(
                    pAudioSource->frameIndexMin(),
                    kFingerprintDuration * pAudioSource->sampleRate()));
    mixxx::AudioSourceStereoProxy audioSourceProxy(
            pAudioSource,
            fingerprintRange.length());

    return calcFingerprint(audioSourceProxy, fingerprintRange);
}
