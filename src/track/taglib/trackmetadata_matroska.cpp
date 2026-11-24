#if defined(_MSC_VER)
#pragma warning(push)
// https://github.com/taglib/taglib/issues/1185
// warning C4251: 'TagLib::FileName::m_wname': class
// 'std::basic_string<wchar_t,std::char_traits<wchar_t>,std::allocator<wchar_t>>'
// needs to have dll-interface to be used by clients of class 'TagLib::FileName'
#pragma warning(disable : 4251)
#endif

#include "track/taglib/trackmetadata_matroska.h"

#if (TAGLIB_MAJOR_VERSION > 2) || \
        ((TAGLIB_MAJOR_VERSION == 2) && (TAGLIB_MINOR_VERSION >= 2))
#include "track/taglib/trackmetadata_common.h"
#include "util/logger.h"

namespace mixxx {

namespace {

Logger kLogger("TagLib");

} // anonymous namespace

namespace taglib {

namespace matroska {

bool importCoverImageFromTag(
        QImage* pCoverArt,
        const TagLib::Matroska::File& file) {
    if (!pCoverArt) {
        return false; // nothing to do
    }

    const auto pictures = file.complexProperties("PICTURE");
    if (pictures.isEmpty()) {
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "No cover art: None or empty list of Matroska attachments";
        }
        return false; // abort
    }

    for (const auto& picture : pictures) {
        const auto data = picture.value("data").toByteVector();
        const auto mimeType = picture.value("mimeType").toString();
        if (data.isEmpty()) {
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Skipping empty Matroska cover art attachment";
            }
            continue;
        }
        const QImage image(loadImageFromByteVector(
                data,
                mimeType.toCString()));
        if (image.isNull()) {
            kLogger.warning()
                    << "Failed to load image from Matroska cover art attachment of type"
                    << mimeType.toCString();
            continue;
        }
        *pCoverArt = image;
        return true; // success
    }

    kLogger.warning()
            << "Failed to load cover art image from Matroska attachments";
    return false;
}

} // namespace matroska

} // namespace taglib

} // namespace mixxx

#endif // TagLib >= 2.2
