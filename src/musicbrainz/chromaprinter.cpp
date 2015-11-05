#include <chromaprint.h>

#include <QtDebug>

#include "musicbrainz/chromaprinter.h"
#include "soundsourceproxy.h"

ChromaPrinter::ChromaPrinter(QObject* parent)
             : QObject(parent) {
}

QString ChromaPrinter::getFingerPrint(TrackPointer pTrack) {
    SoundSourceProxy soundSourceProxy(pTrack);
    Mixxx::SoundSourcePointer pSoundSource(soundSourceProxy.open());
    if (pSoundSource.isNull()) {
        qDebug() << "Skipping invalid file:" << pTrack->getLocation();
        return QString();
    }
    if (0 >= pSoundSource->length()) {
        qDebug() << "Skipping empty file:" << pTrack->getLocation();
        return QString();
    }
    return calcFingerPrint(pSoundSource);
}

QString ChromaPrinter::calcFingerPrint(const Mixxx::SoundSourcePointer& pSoundSource) {

    // this is worth 2min of audio, multiply by 2 because we have 2 channels
    // AcoustID only stores a fingerprint for the first two minutes of a song
    // on their server so we need only a fingerprint of the first two minutes
    // --kain88 July 2012
    unsigned long maxFingerprintSamples = 120 * 2 * pSoundSource->getSampleRate();
    unsigned long numFingerprintSamples = math_min(pSoundSource->length(), maxFingerprintSamples);

    SAMPLE *pData = new SAMPLE[numFingerprintSamples];
    QTime timerReadingFile;
    timerReadingFile.start();
    unsigned int read = pSoundSource->read(numFingerprintSamples, pData);

    if (read!=numFingerprintSamples) {
        qDebug() << "oh that's embarrasing I couldn't read the track";
        return QString();
    }
    qDebug("reading file took: %d ms" , timerReadingFile.elapsed());

    ChromaprintContext* ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);
    // we have 2 channels in mixxx always
    chromaprint_start(ctx, pSoundSource->getSampleRate(), 2);

    QTime timerGeneratingFingerPrint;
    timerGeneratingFingerPrint.start();
    int success = chromaprint_feed(ctx, pData, numFingerprintSamples);
    if (!success) {
        qDebug() << "could not generate fingerprint";
        delete [] pData;
        return QString();
    }
    chromaprint_finish(ctx);

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
    delete [] pData;

    qDebug("generating fingerprint took: %d ms" , timerGeneratingFingerPrint.elapsed());

    return fingerprint;
}
