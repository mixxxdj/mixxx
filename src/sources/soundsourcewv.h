#ifndef MIXXX_SOUNDSOURCEWV_H
#define MIXXX_SOUNDSOURCEWV_H

#include "sources/soundsource.h"
#include "sources/soundsourceprovider.h"

class QFile;

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
            WritableSampleFrames sampleFrames) override;

  private:
    OpenResult tryOpen(
            OpenMode mode,
            const OpenParams& params) override;

    // A WavpackContext* type
    // we cannot use the type directly, because it has 
    // changing definitions with different wavpack.h versions. 
    // wavpack.h can't be included here, bacause it has concurrent definitions 
    // with other decoder's header.     
    void* m_wpc;
 
    CSAMPLE m_sampleScaleFactor;
    QFile* m_pWVFile;
    QFile* m_pWVCFile;

    SINT m_curFrameIndex;
};

class SoundSourceProviderWV : public SoundSourceProvider {
  public:
    QString getName() const override;

    QStringList getSupportedFileExtensions() const override;

    SoundSourceProviderPriority getPriorityHint(
            const QString& supportedFileExtension) const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override;
};

} // namespace mixxx

#endif // MIXXX_SOUNDSOURCEWV_H
