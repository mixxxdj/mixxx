#pragma once

#include <QAtomicInt>
#include <QByteArray>
#include <QMutex>
#include <QSharedPointer>
#include <QString>
#include <vector>

#include "analyzer/constants.h"
#include "audio/signalinfo.h"
#include "util/class.h"
#include "util/compatibility/qmutex.h"

enum BandIndex { AllBand = 0,
    Low = 1,
    Mid = 2,
    High = 3,
    BandCount = 4 };
enum ChannelIndex { Left = 0, Right = 1, ChannelCount = 2};

struct WaveformFilteredData {
    unsigned char low;
    unsigned char mid;
    unsigned char high;
    unsigned char all;
};

struct WaveformData {
    WaveformFilteredData filtered;
    unsigned char stems[mixxx::kMaxSupportedStems];
};

class Waveform {
  public:
    enum class SaveState {
        NotSaved = 0,
        SavePending,
        Saved
    };

    explicit Waveform(const QByteArray& pData = QByteArray());
    Waveform(
            int audioSampleRate,
            SINT frameLength,
            int desiredVisualSampleRate,
            int maxVisualSamples,
            int stemCount);

    virtual ~Waveform();

    int getId() const {
        const auto locker = lockMutex(&m_mutex);
        return m_id;
    }

    void setId(int id) {
        const auto locker = lockMutex(&m_mutex);
        m_id = id;
    }

    QString getVersion() const {
        const auto locker = lockMutex(&m_mutex);
        return m_version;
    }

    void setVersion(const QString& version) {
        const auto locker = lockMutex(&m_mutex);
        m_version = version;
    }

    QString getDescription() const {
        const auto locker = lockMutex(&m_mutex);
        return m_description;
    }

    void setDescription(const QString& description) {
        const auto locker = lockMutex(&m_mutex);
        m_description = description;
    }

    QByteArray toByteArray() const;

    SaveState saveState() const {
        return m_saveState;
    }

    // AnalysisDAO needs to be able to change the state to savePending when finished
    // so we mark this as const and m_saveState mutable.
    void setSaveState(SaveState eState) const {
        m_saveState = eState;
    }

    // We do not lock the mutex since m_audioVisualRatio is not changed after
    // the constructor runs.
    double getAudioVisualRatio() const {
        return m_audioVisualRatio;
    }

    // Atomically lookup the completion of the waveform. Represents the number
    // of data elements that have been processed out of dataSize.
    int getCompletion() const {
        return m_completion.loadAcquire();
    }
    void setCompletion(int completion) {
        m_completion = completion;
    }

    // We do not lock the mutex since m_textureStride is not changed after
    // the constructor runs.
    inline int getTextureStride() const { return m_textureStride; }

    // We do not lock the mutex since m_data is not resized after the
    // constructor runs.
    inline int getTextureSize() const { return static_cast<int>(m_data.size()); }

    // Atomically get the number of data elements in this Waveform. We do not
    // lock the mutex since m_dataSize is not changed after the constructor
    // runs.
    inline int getDataSize() const { return m_dataSize; }

    inline const WaveformData& get(int i) const { return m_data[i];}
    inline unsigned char getLow(int i) const { return m_data[i].filtered.low;}
    inline unsigned char getMid(int i) const { return m_data[i].filtered.mid;}
    inline unsigned char getHigh(int i) const { return m_data[i].filtered.high;}
    inline unsigned char getAll(int i) const { return m_data[i].filtered.all;}

    // We do not lock the mutex since m_data is not resized after the
    // constructor runs.
    WaveformData* data() { return &m_data[0];}

    // We do not lock the mutex since m_data is not resized after the
    // constructor runs.
    const WaveformData* data() const { return &m_data[0];}

    bool hasStem() const {
        return m_stemCount > 0;
    }

    void dump() const;

  private:
    void readByteArray(const QByteArray& data);
    void resize(int size);
    void assign(int size);

    inline WaveformData& at(int i) { return m_data[i];}
    inline unsigned char& low(int i) { return m_data[i].filtered.low;}
    inline unsigned char& mid(int i) { return m_data[i].filtered.mid;}
    inline unsigned char& high(int i) { return m_data[i].filtered.high;}
    inline unsigned char& all(int i) { return m_data[i].filtered.all;}
    double getVisualSampleRate() const { return m_visualSampleRate; }

    // If stored in the database, the ID of the waveform.
    int m_id;
    // mutable since AnalysisDAO needs to be able to set the waveform as saved.
    mutable SaveState m_saveState;
    QString m_version;
    QString m_description;

    // The size of the waveform data stored in m_data. Not allowed to change
    // after the constructor runs.
    int m_dataSize;
    // The vector storing the waveform data. It is potentially larger than
    // m_dataSize since it includes padding for uploading the entire waveform as
    // a texture in the GLSL renderer. The size is not allowed to change after
    // the constructor runs. We use a std::vector to avoid the cost of bounds
    // checking when accessing the vector.
    // TODO(XXX): In the future we should switch to QVector and use the raw data
    // pointer when performance matters.
    std::vector<WaveformData> m_data;
    // Not allowed to change after the constructor runs.
    double m_visualSampleRate;
    // Not allowed to change after the constructor runs.
    double m_audioVisualRatio;

    // We create an NxN texture out of m_data's buffer in the GLSL renderer. The
    // stride is N. Not allowed to change after the constructor runs.
    int m_textureStride;

    // For performance, completion is shared as a QAtomicInt and does not lock
    // the mutex. The completion of the waveform calculation.
    QAtomicInt m_completion;

    // The number of stem contained in waveform samples. 0 if not a stem waveform
    int m_stemCount;

    mutable QMutex m_mutex;

    DISALLOW_COPY_AND_ASSIGN(Waveform);
};

typedef QSharedPointer<Waveform> WaveformPointer;
typedef QSharedPointer<const Waveform> ConstWaveformPointer;
