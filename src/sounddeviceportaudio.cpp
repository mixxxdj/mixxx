/***************************************************************************
                          sounddeviceportaudio.cpp
                             -------------------
    begin                : Sun Aug 15, 2007 (Stardate -315378.5417935057)
    copyright            : (C) 2007 Albert Santoni
    email                : gamegod \a\t users.sf.net
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <portaudio.h>

#include <QtDebug>
#include <QThread>

#ifdef __LINUX__
#include <QLibrary>
#endif

#include "sounddeviceportaudio.h"

#include "soundmanager.h"
#include "sounddevice.h"
#include "soundmanagerutil.h"
#include "controlobject.h"
#include "visualplayposition.h"
#include "util/timer.h"
#include "util/trace.h"
#include "vinylcontrol/defs_vinylcontrol.h"
#include "sampleutil.h"

// static
volatile int SoundDevicePortAudio::m_underflowHappend = 0;

SoundDevicePortAudio::SoundDevicePortAudio(ConfigObject<ConfigValue> *config, SoundManager *sm,
                                           const PaDeviceInfo *deviceInfo, unsigned int devIndex)
        : SoundDevice(config, sm),
          m_pStream(NULL),
          m_devId(devIndex),
          m_deviceInfo(deviceInfo),
          m_outputFifo(NULL),
          m_inputFifo(NULL),
          m_outputDrift(false),
          m_inputDrift(false),
          m_bSetThreadPriority(false),
          m_pMasterUnderflowCount(ControlObject::getControl(
              ConfigKey("[Master]", "underflow_count"))),
          m_underflowUpdateCount(0) {
    // Setting parent class members:
    m_hostAPI = Pa_GetHostApiInfo(deviceInfo->hostApi)->name;
    m_dSampleRate = deviceInfo->defaultSampleRate;
    m_strInternalName = QString("%1, %2").arg(QString::number(m_devId), deviceInfo->name);
    m_strDisplayName = QString(deviceInfo->name);
    m_iNumInputChannels = m_deviceInfo->maxInputChannels;
    m_iNumOutputChannels = m_deviceInfo->maxOutputChannels;
}

SoundDevicePortAudio::~SoundDevicePortAudio() {
}

int SoundDevicePortAudio::open(bool isClkRefDevice) {
    qDebug() << "SoundDevicePortAudio::open()" << getInternalName();
    PaError err;

    if (m_audioOutputs.empty() && m_audioInputs.empty()) {
        m_lastError = QString::fromAscii("No inputs or outputs in SDPA::open() "
            "(THIS IS A BUG, this should be filtered by SM::setupDevices)");
        return ERR;
    }

    memset(&m_outputParams, 0, sizeof(m_outputParams));
    memset(&m_inputParams, 0, sizeof(m_inputParams));
    PaStreamParameters* pOutputParams = &m_outputParams;
    PaStreamParameters* pInputParams = &m_inputParams;

    // Look at how many audio outputs we have,
    // so we can figure out how many output channels we need to open.
    if (m_audioOutputs.empty()) {
        m_outputParams.channelCount = 0;
        pOutputParams = NULL;
    } else {
        foreach (AudioOutput out, m_audioOutputs) {
            ChannelGroup channelGroup = out.getChannelGroup();
            int highChannel = channelGroup.getChannelBase()
                    + channelGroup.getChannelCount();
            if (m_outputParams.channelCount <= highChannel) {
                m_outputParams.channelCount = highChannel;
            }
        }
    }

    // Look at how many audio inputs we have,
    // so we can figure out how many input channels we need to open.
    if (m_audioInputs.empty()) {
        m_inputParams.channelCount = 0;
        pInputParams = NULL;
    } else {
        foreach (AudioInput in, m_audioInputs) {
            ChannelGroup channelGroup = in.getChannelGroup();
            int highChannel = channelGroup.getChannelBase()
                + channelGroup.getChannelCount();
            if (m_inputParams.channelCount <= highChannel) {
                m_inputParams.channelCount = highChannel;
            }
        }
    }

    // Workaround for Bug #900364. The PortAudio ALSA hostapi opens the minimum
    // number of device channels supported by the device regardless of our
    // channel request. It has no way of notifying us when it does this. The
    // typical case this happens is when we are opening a device in mono when it
    // supports a minimum of stereo. To work around this, simply open the device
    // in stereo and only take the first channel.
    // TODO(rryan): Remove once PortAudio has a solution built in (and
    // released).
    if (m_deviceInfo->hostApi == paALSA) {
        // Only engage workaround if the device has enough input and output
        // channels.
        if (m_deviceInfo->maxInputChannels >= 2 &&
                m_inputParams.channelCount == 1) {
            m_inputParams.channelCount = 2;
        }
        if (m_deviceInfo->maxOutputChannels >= 2 &&
                m_outputParams.channelCount == 1) {
            m_outputParams.channelCount = 2;
        }
    }


    // Sample rate
    if (m_dSampleRate <= 0) {
        m_dSampleRate = 44100.0;
    }

    // Get latency in milleseconds
    qDebug() << "framesPerBuffer:" << m_framesPerBuffer;
    double bufferMSec = m_framesPerBuffer / m_dSampleRate * 1000;
    qDebug() << "Requested sample rate: " << m_dSampleRate << "Hz, latency:" << bufferMSec << "ms";

    qDebug() << "Output channels:" << m_outputParams.channelCount << "| Input channels:"
        << m_inputParams.channelCount;

    // PortAudio's JACK backend also only properly supports
    // paFramesPerBufferUnspecified in non-blocking mode because the latency
    // comes from the JACK daemon. (PA should give an error or something though,
    // but it doesn't.)
    if (m_deviceInfo->hostApi == paJACK) {
        m_framesPerBuffer = paFramesPerBufferUnspecified;
    }

    //Fill out the rest of the info.
    m_outputParams.device = m_devId;
    m_outputParams.sampleFormat = paFloat32;
    m_outputParams.suggestedLatency = bufferMSec / 1000.0;
    m_outputParams.hostApiSpecificStreamInfo = NULL;

    m_inputParams.device  = m_devId;
    m_inputParams.sampleFormat  = paFloat32;
    m_inputParams.suggestedLatency = bufferMSec / 1000.0;
    m_inputParams.hostApiSpecificStreamInfo = NULL;

    qDebug() << "Opening stream with id" << m_devId;

    //Create the callback function pointer.
    PaStreamCallback* callback = NULL;
    if (isClkRefDevice) {
        callback = paV19CallbackClkRef;
    } else {
        callback = paV19Callback;
        // to avoid overflows when one callback overtakes the other or
        // when there is a clock drift compared to the clock reference device
        // we need an additional artificial delay
        if (m_outputParams.channelCount) {
            // On chunk for reading one for writing and on for drift correction
            m_outputFifo = new FIFO<CSAMPLE>(m_outputParams.channelCount * m_framesPerBuffer * 3);
            // Clear first 1.5 chunks on for the required artificial delaly to a allow jitter
            // and a half, because we can't predict which callback fires first.
            m_outputFifo->releaseWriteRegions(m_outputParams.channelCount * m_framesPerBuffer * 3 / 2);
        }
        if (m_inputParams.channelCount) {
            m_inputFifo = new FIFO<CSAMPLE>(m_inputParams.channelCount * m_framesPerBuffer * 3);
            // Clear first two 1.5 chunks (see above)
            m_inputFifo->releaseWriteRegions(m_inputParams.channelCount * m_framesPerBuffer * 3 / 2);
        }
    }

    PaStream *pStream;
    // Try open device using iChannelMax
    err = Pa_OpenStream(&pStream,
                        pInputParams,
                        pOutputParams,
                        m_dSampleRate,
                        m_framesPerBuffer,
                        paClipOff, // Stream flags
                        callback,
                        (void*) this); // pointer passed to the callback function

    if (err != paNoError) {
        qWarning() << "Error opening stream:" << Pa_GetErrorText(err);
        m_lastError = QString::fromUtf8(Pa_GetErrorText(err));
        return ERR;
    } else {
        qDebug() << "Opened PortAudio stream successfully... starting";
    }


#ifdef __LINUX__
    //Attempt to dynamically load and resolve stuff in the PortAudio library
    //in order to enable RT priority with ALSA.
    QLibrary portaudio("libportaudio.so.2");
    if (!portaudio.load())
       qWarning() << "Failed to dynamically load PortAudio library";
    else
       qDebug() << "Dynamically loaded PortAudio library";

    EnableAlsaRT enableRealtime = (EnableAlsaRT) portaudio.resolve("PaAlsa_EnableRealtimeScheduling");
    if (enableRealtime) {
        enableRealtime(pStream, 1);
    }
    portaudio.unload();
#endif

    // Start stream
    err = Pa_StartStream(pStream);
    if (err != paNoError) {
        qWarning() << "PortAudio: Start stream error:" << Pa_GetErrorText(err);
        m_lastError = QString::fromUtf8(Pa_GetErrorText(err));
        err = Pa_CloseStream(pStream);
        if (err != paNoError) {
            qWarning() << "PortAudio: Close stream error:" << Pa_GetErrorText(err) << getInternalName();
        }
        return ERR;
    } else {
        qDebug() << "PortAudio: Started stream successfully";
    }

    // Get the actual details of the stream & update Mixxx's data
    const PaStreamInfo* streamDetails = Pa_GetStreamInfo(pStream);
    m_dSampleRate = streamDetails->sampleRate;
    double currentLatencyMSec = streamDetails->outputLatency * 1000;
    qDebug() << "   Actual sample rate: " << m_dSampleRate << "Hz, latency:" << currentLatencyMSec << "ms";

    if (isClkRefDevice) {
        // Update the samplerate and latency ControlObjects, which allow the
        // waveform view to properly correct for the latency.
        ControlObject::set(ConfigKey("[Master]", "latency"), currentLatencyMSec);
        ControlObject::set(ConfigKey("[Master]", "samplerate"), m_dSampleRate);
        ControlObject::set(ConfigKey("[Master]", "audio_buffer_size"), bufferMSec);

        if (m_pMasterUnderflowCount) {
            m_pMasterUnderflowCount->set(0);
        }
    }
    m_pStream = pStream;
    return OK;
}

int SoundDevicePortAudio::close() {
    //qDebug() << "SoundDevicePortAudio::close()" << getInternalName();
    PaStream* pStream = m_pStream;
    m_pStream = NULL;
    if (pStream) {
        // Make sure the stream is not stopped before we try stopping it.
        PaError err = Pa_IsStreamStopped(pStream);
        // 1 means the stream is stopped. 0 means active.
        if (err == 1) {
            qDebug() << "PortAudio: Stream already stopped, but no error.";
            return 1;
        }
        // Real PaErrors are always negative.
        if (err < 0) {
            qWarning() << "PortAudio: Stream already stopped:"
                       << Pa_GetErrorText(err) << getInternalName();
            return 1;
        }

        //Stop the stream.
        err = Pa_StopStream(pStream);
        //PaError err = Pa_AbortStream(m_pStream); //Trying Pa_AbortStream instead, because StopStream seems to wait
                                                   //until all the buffers have been flushed, which can take a
                                                   //few (annoying) seconds when you're doing soundcard input.
                                                   //(it flushes the input buffer, and then some, or something)
                                                   //BIG FAT WARNING: Pa_AbortStream() will kill threads while they're
                                                   //waiting on a mutex, which will leave the mutex in an screwy
                                                   //state. Don't use it!

        if (err != paNoError) {
            qWarning() << "PortAudio: Stop stream error:" << Pa_GetErrorText(err) << getInternalName();
            return 1;
        }

        // Close stream
        err = Pa_CloseStream(pStream);
        if (err != paNoError) {
            qWarning() << "PortAudio: Close stream error:" << Pa_GetErrorText(err) << getInternalName();
            return 1;
        }

        if (m_outputFifo) {
            delete m_outputFifo;
        }
        if (m_inputFifo) {
            delete m_inputFifo;
        }
    }

    m_outputFifo = NULL;
    m_inputFifo = NULL;
    m_bSetThreadPriority = false;

    return 0;
}

QString SoundDevicePortAudio::getError() const {
    return m_lastError;
}

void SoundDevicePortAudio::readProcess() {
    PaStream* pStream = m_pStream;
    if (pStream && m_inputParams.channelCount) {
        int inChunkSize = m_framesPerBuffer * m_inputParams.channelCount;
        int readAvailable = m_inputFifo->readAvailable();
        int readCount = inChunkSize;
        if (inChunkSize > readAvailable) {
            readCount = readAvailable;
            m_underflowHappend = 1;
            //qDebug() << "readProcess()" << (float)readAvailable / inChunkSize << "underflow";
        }
        if (readCount) {
            CSAMPLE* dataPtr1;
            ring_buffer_size_t size1;
            CSAMPLE* dataPtr2;
            ring_buffer_size_t size2;
            // We use size1 and size2, so we can ignore the return value
            (void)m_inputFifo->aquireReadRegions(readCount, &dataPtr1, &size1, &dataPtr2, &size2);
            // Fetch fresh samples and write to the the output buffer
            composeInputBuffer(dataPtr1,
                    size1 / m_inputParams.channelCount, 0,
                    m_inputParams.channelCount);
            if (size2 > 0) {
                composeInputBuffer(dataPtr2,
                        size2 / m_inputParams.channelCount, size1 / m_inputParams.channelCount,
                        m_inputParams.channelCount);
            }
            m_inputFifo->releaseReadRegions(readCount);
        }
        if (readCount < inChunkSize) {
            // Fill remaining buffers with zeros
            clearInputBuffer(inChunkSize - readCount, readCount);
        }

        m_pSoundManager->pushInputBuffers(m_audioInputs, m_framesPerBuffer);
    }
}

void SoundDevicePortAudio::writeProcess() {
    PaStream* pStream = m_pStream;
    if (pStream && m_outputParams.channelCount) {
        int outChunkSize = m_framesPerBuffer * m_outputParams.channelCount;
        int writeAvailable = m_outputFifo->writeAvailable();
        int writeCount = outChunkSize;
        if (outChunkSize > writeAvailable) {
            writeCount = writeAvailable;
            m_underflowHappend = 1;
            //qDebug() << "writeProcess():" << (float) readAvailable / outChunkSize << "Overflow";
        }
        if (writeCount) {
            CSAMPLE* dataPtr1;
            ring_buffer_size_t size1;
            CSAMPLE* dataPtr2;
            ring_buffer_size_t size2;
            // We use size1 and size2, so we can ignore the return value
            (void)m_outputFifo->aquireWriteRegions(writeCount, &dataPtr1, &size1, &dataPtr2, &size2);
            // Fetch fresh samples and write to the the output buffer
            composeOutputBuffer(dataPtr1, size1 / m_outputParams.channelCount, 0,
                    static_cast<unsigned int>(m_outputParams.channelCount));
            if (size2 > 0) {
                composeOutputBuffer(dataPtr2, size2 / m_outputParams.channelCount, size1 / m_outputParams.channelCount,
                        static_cast<unsigned int>(m_outputParams.channelCount));
            }
            m_outputFifo->releaseWriteRegions(writeCount);
        }
    }
}

int SoundDevicePortAudio::callbackProcess(const unsigned int framesPerBuffer,
                                          CSAMPLE *out, const CSAMPLE *in,
                                          const PaStreamCallbackTimeInfo *timeInfo,
                                          PaStreamCallbackFlags statusFlags) {
    Q_UNUSED(timeInfo);
    Trace trace("SoundDevicePortAudio::callbackProcess " + getInternalName());

    //qDebug() << "SoundDevicePortAudio::callbackProcess:" << getInternalName();
    // Turn on TimeCritical priority for the callback thread. If we are running
    // in Linux userland, for example, this will have no effect.
    if (!m_bSetThreadPriority) {
        QThread::currentThread()->setPriority(QThread::TimeCriticalPriority);
        m_bSetThreadPriority = true;
    }

    if (statusFlags & (paOutputUnderflow | paInputOverflow)) {
        m_underflowHappend = 1;
    }

    if (m_inputParams.channelCount) {
        int inChunkSize = framesPerBuffer * m_inputParams.channelCount;
        int readAvailable = m_inputFifo->readAvailable();
        int writeAvailable = m_inputFifo->writeAvailable();
        if (readAvailable < inChunkSize) {
            // risk of an underflow, duplicate one sample
            m_inputFifo->write(in, inChunkSize);
            if (m_inputDrift) {
                // Do not compensate the first delay, because it is likely a jitter
                // corrected in the next cycle
                // Duplicate one sample
                m_inputFifo->write(&in[inChunkSize - m_inputParams.channelCount], m_inputParams.channelCount);
                //qDebug() << "callbackProcess write:" << (float)readAvailable / inChunkSize << "Skip";
            } else {
                m_inputDrift = true;
                //qDebug() << "callbackProcess write:" << (float)readAvailable / inChunkSize << "Jitter Skip";
            }
        } else if (readAvailable == inChunkSize) {
            // Everything Ok
            m_inputFifo->write(in, inChunkSize);
            m_inputDrift = false;
            //qDebug() << "callbackProcess write:" << (float) readAvailable / inChunkSize << "Normal";
        } else if (writeAvailable >= inChunkSize) {
            // Risk of overflow, save one sample
            if (m_inputDrift) {
                m_inputFifo->write(in, inChunkSize - m_inputParams.channelCount);
                //qDebug() << "callbackProcess write:" << (float)readAvailable / inChunkSize << "Save";
            } else {
                m_inputFifo->write(in, inChunkSize);
                m_inputDrift = true;
                //qDebug() << "callbackProcess write:" << (float)readAvailable / inChunkSize << "Jitter Save";
            }
        } else if (writeAvailable) {
            // Fifo Overflow
            m_inputFifo->write(in, writeAvailable);
            m_underflowHappend = 1;
            //qDebug() << "callbackProcess write:" << (float) readAvailable / inChunkSize << "Overflow";
        } else {
            // Buffer full
            m_underflowHappend = 1;
            //qDebug() << "callbackProcess write:" << (float) readAvailable / inChunkSize << "Buffer full";
        }
    }

    if (m_outputParams.channelCount) {
        int outChunkSize = framesPerBuffer * m_outputParams.channelCount;
        int readAvailable = m_outputFifo->readAvailable();

        if (readAvailable > outChunkSize * 2) {
            m_outputFifo->read(out, outChunkSize);
            if (m_outputDrift) {
                m_outputFifo->releaseReadRegions(m_outputParams.channelCount);
                //qDebug() << "callbackProcess read:" << (float)readAvailable / outChunkSize << "Skip";
            } else {
                m_outputDrift = true;
                //qDebug() << "callbackProcess read:" << (float)readAvailable / outChunkSize << "Jitter Skip";
            }
        } else if (readAvailable == outChunkSize * 2) {
            m_outputFifo->read(out,outChunkSize);
            m_outputDrift = false;
            //qDebug() << "callbackProcess read:" << (float)readAvailable / outChunkSize << "Normal";
        } else if (readAvailable >= outChunkSize) {
            // Risk of underflow, save one sample
            if (m_outputDrift) {
                m_outputFifo->read(out, outChunkSize - m_outputParams.channelCount);
                memcpy(&out[outChunkSize - m_outputParams.channelCount],
                       &out[outChunkSize - (2 * m_outputParams.channelCount)],
                        m_outputParams.channelCount * sizeof(CSAMPLE));
                //qDebug() << "callbackProcess read:" << (float)readAvailable / outChunkSize << "Save";
            } else {
                m_outputFifo->read(out, outChunkSize);
                m_outputDrift = true;
                //qDebug() << "callbackProcess read:" << (float)readAvailable / outChunkSize << "Jitter Save";
            }
        } else if (readAvailable) {
            m_outputFifo->read(out,
                    readAvailable);
            // underflow
            SampleUtil::clear(&out[readAvailable],
                    outChunkSize - readAvailable);
            m_underflowHappend = 1;
            //qDebug() << "callbackProcess read:" << (float)readAvailable / outChunkSize << "undeflow";
        } else {
            // underflow
            SampleUtil::clear(out, outChunkSize);
            m_underflowHappend = 1;
            //qDebug() << "callbackProcess read:" << (float)readAvailable / outChunkSize << "Buffer empty";
        }
     }
    return paContinue;
}

int SoundDevicePortAudio::callbackProcessClkRef(const unsigned int framesPerBuffer,
                                          CSAMPLE *out, const CSAMPLE *in,
                                          const PaStreamCallbackTimeInfo *timeInfo,
                                          PaStreamCallbackFlags statusFlags) {
    Trace trace("SoundDevicePortAudio::callbackProcess " + getInternalName());

    //qDebug() << "SoundDevicePortAudio::callbackProcess:" << getInternalName();
    // Turn on TimeCritical priority for the callback thread. If we are running
    // in Linux userland, for example, this will have no effect.
    if (!m_bSetThreadPriority) {
        QThread::currentThread()->setPriority(QThread::TimeCriticalPriority);
        m_bSetThreadPriority = true;
    }

    VisualPlayPosition::setTimeInfo(timeInfo);

    if (!m_underflowUpdateCount) {
        if ((statusFlags & (paOutputUnderflow | paInputOverflow)) ||
                m_underflowHappend) {
            if (m_pMasterUnderflowCount) {
                m_pMasterUnderflowCount->set(
                    m_pMasterUnderflowCount->get() + 1);
            }
            m_underflowUpdateCount = 40;
            m_underflowHappend = 0; // reseting her is not thread save,
                                    // but that is OK, because we count only
                                    // 1/40 of underflows
        }
    } else {
        m_underflowUpdateCount--;
    }

    //Note: Input is processed first so that any ControlObject changes made in
    //      response to input are processed as soon as possible (that is, when
    //      m_pSoundManager->requestBuffer() is called below.)

    // Send audio from the soundcard's input off to the SoundManager...
    if (in) {
        ScopedTimer t("SoundDevicePortAudio::callbackProcess input " + getInternalName());
        composeInputBuffer(in, framesPerBuffer, 0,
                           m_inputParams.channelCount);
        m_pSoundManager->pushInputBuffers(m_audioInputs, m_framesPerBuffer);
    }

    m_pSoundManager->readProcess();

    {
        ScopedTimer t("SoundDevicePortAudio::callbackProcess prepare " + getInternalName());
        m_pSoundManager->onDeviceOutputCallback(framesPerBuffer);
    }

    if (out) {
        ScopedTimer t("SoundDevicePortAudio::callbackProcess output " + getInternalName());

        if (m_outputParams.channelCount <= 0) {
            qWarning() << "SoundDevicePortAudio::callbackProcess m_outputParams channel count is zero or less:" << m_outputParams.channelCount;
            // Bail out.
            return paContinue;
        }

        composeOutputBuffer(out, framesPerBuffer, 0, static_cast<unsigned int>(
                m_outputParams.channelCount));
    }

    m_pSoundManager->writeProcess();

    return paContinue;
}

int paV19Callback(const void *inputBuffer, void *outputBuffer,
                  unsigned long framesPerBuffer,
                  const PaStreamCallbackTimeInfo *timeInfo,
                  PaStreamCallbackFlags statusFlags,
                  void *soundDevice) {
    return ((SoundDevicePortAudio*)soundDevice)->callbackProcess((unsigned int)framesPerBuffer,
            (CSAMPLE*)outputBuffer, (const CSAMPLE*)inputBuffer, timeInfo, statusFlags);
}

int paV19CallbackClkRef(const void *inputBuffer, void *outputBuffer,
                  unsigned long framesPerBuffer,
                  const PaStreamCallbackTimeInfo *timeInfo,
                  PaStreamCallbackFlags statusFlags,
                  void *soundDevice) {
    return ((SoundDevicePortAudio*)soundDevice)->callbackProcessClkRef((unsigned int)framesPerBuffer,
            (CSAMPLE*)outputBuffer, (const CSAMPLE*)inputBuffer, timeInfo, statusFlags);
}
