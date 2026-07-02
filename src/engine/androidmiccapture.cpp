#include "engine/androidmiccapture.h"

#include <QDebug>
#include <QJniEnvironment>

#ifdef Q_OS_ANDROID
#include <QNativeInterface/QAndroidApplication>
#endif

AndroidMicCapture::AndroidMicCapture(QObject* parent)
        : QThread(parent),
          m_framesCaptured(0),
          m_framesConsumed(0),
          m_running(0),
          m_sampleRate(48000) {
    m_circularBuffer.resize(kBufferSize);
    m_circularBuffer.fill(0.0f);
}

AndroidMicCapture::~AndroidMicCapture() {
    stopCapture();
    wait(1000);
}

bool AndroidMicCapture::startCapture(int sampleRate) {
#ifdef Q_OS_ANDROID
    m_sampleRate = sampleRate;
    m_framesCaptured.storeRelaxed(0);
    m_framesConsumed.storeRelaxed(0);
    m_circularBuffer.fill(0.0f);

    openAudioRecord(sampleRate);
    if (!m_audioRecord.isValid()) {
        emit captureFailed(tr("Failed to create AudioRecord"));
        return false;
    }

    // Start recording
    m_audioRecord.callMethod<void>("startRecording");
    m_running.storeRelaxed(1);

    // Start the reader thread
    start();
    qDebug() << "AndroidMicCapture: started at" << sampleRate << "Hz";
    return true;
#else
    Q_UNUSED(sampleRate);
    qDebug() << "AndroidMicCapture: not on Android, returning false";
    return false;
#endif
}

void AndroidMicCapture::stopCapture() {
#ifdef Q_OS_ANDROID
    m_running.storeRelaxed(0);
    // Wait for thread to finish
    wait(2000);
    closeAudioRecord();
    qDebug() << "AndroidMicCapture: stopped, captured"
             << m_framesCaptured.loadRelaxed() << "frames";
#endif
}

int AndroidMicCapture::readFrames(CSAMPLE* buffer, int maxFrames) {
    int captured = m_framesCaptured.loadAcquire();
    int consumed = m_framesConsumed.loadAcquire();
    int available = captured - consumed;
    if (available <= 0) {
        return 0;
    }
    int toRead = qMin(available, maxFrames);
    if (toRead <= 0) {
        return 0;
    }
    for (int i = 0; i < toRead; ++i) {
        int idx = (consumed + i) % kBufferSize;
        buffer[i] = m_circularBuffer[idx];
    }
    m_framesConsumed.storeRelease(consumed + toRead);
    return toRead;
}

void AndroidMicCapture::run() {
#ifdef Q_OS_ANDROID
    // Buffer for reading from AudioRecord (short ints, PCM 16-bit)
    const int readSize = 4096;
    jshortArray jBuffer = QJniEnvironment().newShortArray(readSize);
    if (!jBuffer) {
        qWarning() << "AndroidMicCapture: failed to allocate JNI buffer";
        return;
    }

    while (m_running.loadAcquire()) {
        // Read from AudioRecord (returns short int count)
        jint framesRead = m_audioRecord.callMethod<jint>(
                "read", "([SII)I", jBuffer, 0, readSize);

        if (framesRead > 0) {
            // Get the short array from JNI
            jshort* nativeBuffer = QJniEnvironment().getShortArrayElements(
                    jBuffer, nullptr);
            if (nativeBuffer) {
                for (int i = 0; i < framesRead; ++i) {
                    int idx = (m_framesCaptured.loadRelaxed() + i) % kBufferSize;
                    // Convert PCM 16-bit short to float [-1.0, 1.0]
                    m_circularBuffer[idx] =
                            static_cast<CSAMPLE>(nativeBuffer[i]) / 32768.0f;
                }
                QJniEnvironment().releaseShortArrayElements(
                        jBuffer, nativeBuffer, JNI_ABORT);
                m_framesCaptured.fetchAddRelease(framesRead);
            }
        } else if (framesRead < 0) {
            qWarning() << "AndroidMicCapture: AudioRecord read error:"
                       << framesRead;
            // Throttle on error
            msleep(10);
        }
    }

    QJniEnvironment().deleteLocalRef(jBuffer);
#else
    Q_UNUSED(jBuffer); // suppress unused warning in non-Android build
#endif
}

void AndroidMicCapture::openAudioRecord(int sampleRate) {
#ifdef Q_OS_ANDROID
    // Create AudioRecord with VOICE_RECOGNITION source
    // AudioRecord(int audioSource, int sampleRateInHz, int channelConfig,
    //             int audioFormat, int bufferSizeInBytes)
    //
    // Constants (from android.media):
    //   MediaRecorder.AudioSource.VOICE_RECOGNITION = 6
    //   AudioFormat.CHANNEL_IN_MONO = 0x10 (16)
    //   AudioFormat.ENCODING_PCM_16BIT = 2
    //
    // VOICE_RECOGNITION uses the built-in mic, not BT SCO.

    int minBufferSize = QJniObject::callStaticMethod<jint>(
            "android/media/AudioRecord",
            "getMinBufferSize",
            "(III)I",
            sampleRate,
            0x10, // CHANNEL_IN_MONO
            2);   // ENCODING_PCM_16BIT

    if (minBufferSize < 0) {
        qWarning() << "AndroidMicCapture: getMinBufferSize failed:"
                   << minBufferSize;
        emit captureFailed(
                tr("AudioRecord: getMinBufferSize failed. "
                   "Check RECORD_AUDIO permission."));
        return;
    }

    // Use a larger buffer to avoid underflow
    int bufSize = qMax(minBufferSize, sampleRate / 10); // 100ms worth

    qDebug() << "AndroidMicCapture: creating AudioRecord sampleRate="
             << sampleRate << "minBufferSize=" << minBufferSize
             << "usingBufSize=" << bufSize;

    m_audioRecord = QJniObject(
            "android/media/AudioRecord",
            "(IIIII)V",
            6,          // VOICE_RECOGNITION
            sampleRate,
            0x10,       // CHANNEL_IN_MONO
            2,          // ENCODING_PCM_16BIT
            bufSize);

    if (!m_audioRecord.isValid()) {
        qWarning() << "AndroidMicCapture: AudioRecord constructor failed";
        emit captureFailed(tr("Failed to open microphone. "
                              "Is RECORD_AUDIO permission granted?"));
        return;
    }

    // Check state
    jint state = m_audioRecord.callMethod<jint>("getState");
    qDebug() << "AndroidMicCapture: AudioRecord state=" << state
             << "(expect 1=INITIALIZED)";
#else
    Q_UNUSED(sampleRate);
    qDebug() << "AndroidMicCapture::openAudioRecord: not on Android";
#endif
}

void AndroidMicCapture::closeAudioRecord() {
#ifdef Q_OS_ANDROID
    if (m_audioRecord.isValid()) {
        jint state = m_audioRecord.callMethod<jint>("getState");
        if (state == 1) { // INITIALIZED or RECORDING
            m_audioRecord.callMethod<void>("stop");
        }
        m_audioRecord.callMethod<void>("release");
        m_audioRecord = QJniObject(); // invalidate
        qDebug() << "AndroidMicCapture: AudioRecord released";
    }
#else
    // no-op on non-Android
#endif
}