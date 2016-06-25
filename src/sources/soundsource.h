#ifndef MIXXX_SOUNDSOURCE_H
#define MIXXX_SOUNDSOURCE_H

#include "sources/metadatasource.h"
#include "sources/audiosource.h"

namespace mixxx {

// Base class for sound sources.
class SoundSource: public MetadataSource, public AudioSource {
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

    enum class OpenResult {
        SUCCEEDED,
        FAILED,
        // If a SoundSource supports only some of the file formats
        // behind a supported file extension it may return the
        // following error. This is not really a failure as it
        // only indicates a lack of functionality.
        UNSUPPORTED_FORMAT
    };

    // Opens the AudioSource for reading audio data.
    //
    // Since reopening is not supported close() will be called
    // implicitly before the AudioSource is actually opened.
    //
    // Optionally the caller may provide the desired properties
    // of the decoded audio signal. Some decoders are able to reduce
    // the number of channels or do resampling on the fly while decoding
    // the input data.
    OpenResult open(const AudioSourceConfig& audioSrcCfg = AudioSourceConfig());

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

private:
    // Tries to open the AudioSource for reading audio data according
    // to the "Template Method" design pattern.
    //
    // The invocation of tryOpen() is enclosed in invocations of close():
    //   - Before: Always
    //   - After: Upon failure
    // If tryOpen() throws an exception or returns a result other than
    // OpenResult::SUCCEEDED an invocation of close() will follow.
    // Implementations do not need to free internal resources twice in
    // both tryOpen() upon failure and close(). All internal resources
    // should be freed in close() instead.
    //
    // Exceptions should be handled internally by implementations to
    // avoid warning messages about unexpected or unknown exceptions.
    virtual OpenResult tryOpen(const AudioSourceConfig& audioSrcCfg) = 0;

    const QString m_type;
};

typedef QSharedPointer<SoundSource> SoundSourcePointer;

} //namespace mixxx

#endif // MIXXX_SOUNDSOURCE_H
