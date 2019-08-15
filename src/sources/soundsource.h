#pragma once

#include <QDebug>

#include "sources/audiosource.h"
#include "sources/metadatasourcetaglib.h"

#include "util/assert.h"

namespace mixxx {

// Base class for sound sources with a default implementation (Taglib)
// for reading/writing metadata.
class SoundSource
        : public AudioSource,
          public MetadataSourceTagLib {
  public:
    static QString getFileExtensionFromUrl(QUrl url);

    QString getType() const {
        return m_type;
    }

  protected:
    // If no type is provided the file extension of the file referred
    // by the URL will be used as the type of the SoundSource.
    explicit SoundSource(QUrl url)
            : SoundSource(url, getFileExtensionFromUrl(url)) {
    }
    SoundSource(QUrl url, QString type);

  private:
    QString m_type;
};

typedef std::shared_ptr<SoundSource> SoundSourcePointer;

template<typename T>
SoundSourcePointer newSoundSourceFromUrl(QUrl url) {
    return std::make_shared<T>(url);
}

} //namespace mixxx
