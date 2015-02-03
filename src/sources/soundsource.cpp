#include "sources/soundsource.h"
#include "metadata/trackmetadata.h"

namespace Mixxx {

/*static*/ QString SoundSource::getTypeFromUrl(QUrl url) {
    return url.toString().section(".", -1).toLower().trimmed();
}

SoundSource::SoundSource(QUrl url)
        : UrlResource(url),
          m_type(getTypeFromUrl(url)) {
    DEBUG_ASSERT(getUrl().isValid());
}

SoundSource::SoundSource(QUrl url, QString type)
        : UrlResource(url),
          m_type(type) {
    DEBUG_ASSERT(getUrl().isValid());
}

} //namespace Mixxx
