#pragma once

#include "sources/soundsource.h"
#include "sources/soundsourceprovider.h"

QT_FORWARD_DECLARE_CLASS(QFile);

namespace mixxx {

class SoundSourceWV : public SoundSource {
  public:
    static int32_t ReadBytesCallback(void* id, void* data, int bcount);
    static uint32_t GetPosCallback(void* id);
    static int SetPosAbsCallback(void* id, unsigned int pos);
    static int SetPosRelCallback(void* id, int delta, int mode);
    static int PushBackByteCallback(void* id, int c);
    static uint32_t GetlengthCallback(void* id);
    static int CanSeekCallback(void* id);
    static int32_t WriteBytesCallback(void* id, void* data, int32_t bcount);

    explicit SoundSourceWV(const QUrl& url);
    ~SoundSourceWV() override;

    void close() override;

  protected:
    ReadableSampleFrames readSampleFramesClamped(
            const WritableSampleFrames& sampleFrames) override;

  private:
    OpenResult tryOpen(
            OpenMode mode,
            const OpenParams& params) override;

    // A WavpackContext* type
    // we cannot use the type directly, because it has
    // changing definitions with different wavpack.h versions.
    // wavpack.h can't be included here, because it has concurrent definitions
    // with other decoder's header.
    void* m_wpc;

    CSAMPLE m_sampleScaleFactor;
    QFile* m_pWVFile;
    QFile* m_pWVCFile;

    SINT m_curFrameIndex;
};

class SoundSourceProviderWV : public SoundSourceProvider {
  public:
    static const QString kDisplayName;
    static const QStringList kSupportedFileExtensions;

    QString getDisplayName() const override {
        return kDisplayName;
    }

    QStringList getSupportedFileExtensions() const override {
        return kSupportedFileExtensions;
    }

    SoundSourceProviderPriority getPriorityHint(
            const QString& supportedFileExtension) const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override;
};

} // namespace mixxx
