#ifndef MIXXX_SOUNDSOURCE_H
#define MIXXX_SOUNDSOURCE_H

#include <QDebug>

#include "sources/metadatasource.h"
#include "sources/audiosource.h"

#include "util/assert.h"

namespace mixxx {

class ISoundSource {
  public:
    virtual ~ISoundSource() {}

    enum class OpenMode {
        // In Strict mode the opening operation should be aborted
        // as soon as any inconsistencies are detected.
        Strict,
        // Opening in Permissive mode is used only after opening
        // in Strict mode has been aborted by all available
        // SoundSource implementations.
        Permissive,
    };

    enum class OpenResult {
        Succeeded,
        // If a SoundSource is not able to open a file because of
        // internal errors of if the format of the content is not
        // supported it should return Aborted. This gives SoundSources
        // with a lower priority the chance to open the same file.
        // Example: A SoundSourceProvider has been registered for
        // files with a certain extension, but the corresponding
        // SoundSource does only support a subset of all possible
        // data formats that might be stored in files with this
        // extension.
        Aborted,
        // If a SoundSource return Failed while opening a file
        // the entire operation will fail immediately. No other
        // sources with lower priority will be given the chance
        // to open the same file.
        Failed,
    };

  protected:
      // Tries to open the AudioSource for reading audio data according
      // to the "Template Method" design pattern.
      //
      // The invocation of tryOpen() is enclosed in invocations of close():
      //   - Before: Always
      //   - After: Upon failure
      // If tryOpen() throws an exception or returns a result other than
      // OpenResult::Succeeded an invocation of close() will follow.
      // Implementations do not need to free internal resources twice in
      // both tryOpen() upon failure and close(). All internal resources
      // should be freed in close() instead.
      //
      // Exceptions should be handled internally by implementations to
      // avoid warning messages about unexpected or unknown exceptions.
      virtual OpenResult tryOpen(
              OpenMode mode,
              const AudioSourceConfig& audioSrcCfg) = 0;
};

// Base class for sound sources.
class SoundSource: public AudioSource, public virtual ISoundSource, public virtual IMetadataSource {
public:
    static QString getFileExtensionFromUrl(const QUrl& url);

    const QString& getType() const {
        return m_type;
    }

    // Default implementations for reading/writing track metadata.
    Result parseTrackMetadataAndCoverArt(
            TrackMetadata* pTrackMetadata,
            QImage* pCoverArt) const override;
    Result writeTrackMetadata(
            const TrackMetadata& trackMetadata) const override;

    // Opens the AudioSource for reading audio data.
    //
    // Since reopening is not supported close() will be called
    // implicitly before the AudioSource is actually opened.
    //
    // Optionally the caller may provide the desired properties
    // of the decoded audio signal. Some decoders are able to reduce
    // the number of channels or do resampling on the fly while decoding
    // the input data.
    OpenResult open(
            OpenMode mode,
            const AudioSourceConfig& audioSrcCfg = AudioSourceConfig());

    // Closes the AudioSource and frees all resources.
    //
    // Might be called even if the AudioSource has never been
    // opened, has already been closed, or if opening has failed.
    virtual void close() = 0;

protected:
    // If no type is provided the file extension of the file referred
    // by the URL will be used as the type of the SoundSource.
    explicit SoundSource(const QUrl& url);
    SoundSource(const QUrl& url, const QString& type);

    const QString m_type;
};

typedef std::shared_ptr<SoundSource> SoundSourcePointer;

template<typename T>
SoundSourcePointer newSoundSourceFromUrl(const QUrl& url) {
    return std::make_shared<T>(url);
}

inline
QDebug operator<<(QDebug dbg, SoundSource::OpenMode openMode) {
    switch (openMode) {
    case SoundSource::OpenMode::Strict:
        return dbg << "Strict";
    case SoundSource::OpenMode::Permissive:
        return dbg << "Permissive";
    default:
        DEBUG_ASSERT(!"Unknown OpenMode");
        return dbg << "Unknown";
    }
}

inline
QDebug operator<<(QDebug dbg, SoundSource::OpenResult openResult) {
    switch (openResult) {
    case SoundSource::OpenResult::Succeeded:
        return dbg << "Succeeded";
    case SoundSource::OpenResult::Aborted:
        return dbg << "Aborted";
    case SoundSource::OpenResult::Failed:
        return dbg << "Failed";
    default:
        DEBUG_ASSERT(!"Unknown OpenResult");
        return dbg << "Unknown";
    }
}

} //namespace mixxx

#endif // MIXXX_SOUNDSOURCE_H
