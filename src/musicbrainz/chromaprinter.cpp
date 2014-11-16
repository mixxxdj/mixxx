#include "musicbrainz/chromaprinter.h"

#include "soundsourceproxy.h"
#include "sampleutil.h"

#include <chromaprint.h>

#include <QtDebug>


namespace
{
    // this is worth 2min of audio
    // AcoustID only stores a fingerprint for the first two minutes of a song
    // on their server so we need only a fingerprint of the first two minutes
    // --kain88 July 2012
    const Mixxx::AudioSource::size_type kFingerprintDuration = 120; // in seconds

    QString calcFingerPrint(const Mixxx::AudioSourcePointer& pAudioSource) {

        Mixxx::AudioSource::size_type numFrames =
                kFingerprintDuration * pAudioSource->getSampleRate();
        // check that the song is actually longer then the amount of audio we use
        if (numFrames > pAudioSource->getFrameCount()) {
            numFrames = pAudioSource->getFrameCount();
        }

        QTime timerReadingFile;
        timerReadingFile.start();

        const Mixxx::AudioSource::size_type numSamples = pAudioSource->frames2samples(numFrames);
        Mixxx::AudioSource::sample_type* sampleBuffer = new Mixxx::AudioSource::sample_type[numSamples];

        const Mixxx::AudioSource::size_type readFrames = pAudioSource->readFrameSamplesInterleaved(numFrames, sampleBuffer);

        const Mixxx::AudioSource::size_type readSamples = pAudioSource->frames2samples(readFrames);
        SAMPLE *pData = new SAMPLE[pAudioSource->frames2samples(readFrames)];

        for (Mixxx::AudioSource::size_type i = 0; i < readSamples; ++i) {
            pData[i] = SAMPLE(sampleBuffer[i] * SAMPLE_MAX);
        }

        delete[] sampleBuffer;

        if (readFrames != numFrames) {
            qDebug() << "oh that's embarrasing I couldn't read the track";
            delete[] pData;
            return QString();
        }
        qDebug("reading file took: %d ms" , timerReadingFile.elapsed());

        ChromaprintContext* ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);
        // we have 2 channels in mixxx always
        chromaprint_start(ctx, pAudioSource->getSampleRate(), pAudioSource->getChannelCount());

        QTime timerGeneratingFingerPrint;
        timerGeneratingFingerPrint.start();
        int success = chromaprint_feed(ctx, pData, readSamples);
        delete [] pData;
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

        qDebug("generating fingerprint took: %d ms" , timerGeneratingFingerPrint.elapsed());

        return fingerprint;
    }
}

ChromaPrinter::ChromaPrinter(QObject* parent)
             : QObject(parent) {
}

QString ChromaPrinter::getFingerPrint(TrackPointer pTrack) {
    SoundSourceProxy soundSourceProxy(pTrack);
    Mixxx::AudioSourcePointer pAudioSource(soundSourceProxy.open());
    if (pAudioSource.isNull()) {
        qDebug() << "Skipping invalid file:" << pTrack->getLocation();
        return QString();
    }
    if (pAudioSource->isFrameCountEmpty()) {
        qDebug() << "Skipping empty file:" << pTrack->getLocation();
        return QString();
    }
    return calcFingerPrint(pAudioSource);
}
