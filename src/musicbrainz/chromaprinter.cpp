#include "musicbrainz/chromaprinter.h"

#include <chromaprint.h>
#include <vector>

#include <QtDebug>

#include "sources/soundsourceproxy.h"
#include "util/sample.h"
#include "util/samplebuffer.h"
#include "util/performancetimer.h"

namespace
{
    // this is worth 2min of audio
    // AcoustID only stores a fingerprint for the first two minutes of a song
    // on their server so we need only a fingerprint of the first two minutes
    // --kain88 July 2012
    const SINT kFingerprintDuration = 120; // in seconds
    const SINT kFingerprintChannels = mixxx::AudioSource::kChannelCountStereo;

    QString calcFingerprint(const mixxx::AudioSourcePointer& pAudioSource) {

        SINT numFrames =
                kFingerprintDuration * pAudioSource->getSamplingRate();
        // check that the song is actually longer then the amount of audio we use
        if (numFrames > pAudioSource->getFrameCount()) {
            numFrames = pAudioSource->getFrameCount();
        }

        PerformanceTimer timerReadingFile;
        timerReadingFile.start();

        // Allocate a sample buffer with maximum size to avoid the
        // implicit allocation of a temporary buffer when reducing
        // the audio signal to stereo.
        SampleBuffer sampleBuffer(
                math_max(numFrames * kFingerprintChannels, pAudioSource->frames2samples(numFrames)));

        DEBUG_ASSERT(2 == kFingerprintChannels); // implicit assumption of the next line
        const SINT readFrames =
                pAudioSource->readSampleFramesStereo(numFrames, &sampleBuffer);
        if (readFrames != numFrames) {
            qDebug() << "oh that's embarrassing I couldn't read the track";
            return QString();
        }

        std::vector<SAMPLE> fingerprintSamples(readFrames * kFingerprintChannels);
        // Convert floating-point to integer
        SampleUtil::convertFloat32ToS16(
                &fingerprintSamples[0],
                sampleBuffer.data(),
                fingerprintSamples.size());

        qDebug() << "reading file took" << timerReadingFile.elapsed().debugMillisWithUnit();

        ChromaprintContext* ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);
        chromaprint_start(ctx, pAudioSource->getSamplingRate(), kFingerprintChannels);

        PerformanceTimer timerGeneratingFingerprint;
        timerGeneratingFingerprint.start();
        int success = chromaprint_feed(ctx, &fingerprintSamples[0], fingerprintSamples.size());
        chromaprint_finish(ctx);
        if (!success) {
            qDebug() << "could not generate fingerprint";
            chromaprint_free(ctx);
            return QString();
        }

        void* fprint = NULL;
        int size = 0;
        int ret = chromaprint_get_raw_fingerprint(ctx, &fprint, &size);
        QByteArray fingerprint;
        if (ret == 1) {
            void* encoded = NULL;
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
}

ChromaPrinter::ChromaPrinter(QObject* parent)
             : QObject(parent) {
}

QString ChromaPrinter::getFingerprint(TrackPointer pTrack) {
    SoundSourceProxy soundSourceProxy(pTrack);
    mixxx::AudioSourceConfig audioSrcCfg;
    audioSrcCfg.setChannelCount(kFingerprintChannels);
    mixxx::AudioSourcePointer pAudioSource(soundSourceProxy.openAudioSource(audioSrcCfg));
    if (pAudioSource.isNull()) {
        qDebug() << "Skipping invalid file:" << pTrack->getLocation();
        return QString();
    }
    return calcFingerprint(pAudioSource);
}
