#include "track/steminfoimporter.h"

#include <QDataStream>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeDatabase>
#include <QString>
#include <QtEndian>
#include <QtGlobal>

#include "util/logger.h"

namespace mixxx {

namespace {

const mixxx::Logger kLogger("StemInfoImporter");
constexpr int kSupportedStemVersion = 1;
const QStringList kStemMimes = {"audio/mp4", "audio/m4a", "audio/x-m4a", "video/mp4"};
// STEM file are usually detected by probing the specific stem atom contained in
// file, in case the file's MIME is one of the above. In case the MIME detection
// fails, we fallback to match the filename extension with "preferred" file
// extensions
const QStringList kStemPreferredFileExtensions = {".stem.mp4", ".stem.m4a"};
const QColor kStemDefaultColor[] = {
        QColor(0x00, 0x9E, 0x73),
        QColor(0xD5, 0x5E, 0x00),
        QColor(0xCC, 0x79, 0xA7),
        QColor(0x56, 0xB4, 0xE9),
};

struct MP4BoxHeader {
    quint32 size;
    char type[4];
};

constexpr uint32_t kAtomHeaderSize = sizeof(MP4BoxHeader);

/// @brief Deserialise a MP4 atom/box header
/// @param reader the reader to read the box from
/// @param box the box in which to deserialise the data
/// @return the effective box size
quint64 operator>>(QIODevice* reader, MP4BoxHeader& box) {
    reader->read((char*)&box.size, sizeof(box.size));
    // The size is in big endian
    box.size = qFromBigEndian(box.size);
    reader->read(box.type, sizeof(box.type));
    if (box.size == 1) {
        // If the box/atom has a size of 1, it is an extended box, and the true
        // size is in the next 64bits integer big endian
        quint64 extendedSize;
        reader->read((char*)&extendedSize, sizeof(extendedSize));
        box.size = qFromBigEndian(box.size);
        return extendedSize - kAtomHeaderSize - sizeof(extendedSize);
    }
    return box.size - kAtomHeaderSize;
}

const QStringList kStemManifestAtomPath = {
        "moov",
        "udta",
        "stem",
};

/// @brief Seek the reader the atom requested.
/// @param reader The IODevice to search MP4 atom in
/// @param path The list of atom to traverse in the tree, from top to bottom.
/// Must be null terminated
/// @param boxSize The size of the box currently under the cursor to focus the
/// search in
/// @param pathIdx The level of the tree currently search (index of path)
/// @return the size of the box data if found, -1 otherwise
uint32_t seekTillAtom(QIODevice* reader,
        const QStringList& path,
        uint32_t boxSize = 0,
        uint32_t pathIdx = 0) {
    if (static_cast<uint32_t>(path.size()) == pathIdx) {
        return boxSize;
    }

    uint32_t byteRead = 0;
    MP4BoxHeader box;
    quint64 effectiveAtomSize;
    while (!reader->atEnd() && (!boxSize || byteRead < boxSize)) {
        effectiveAtomSize = reader >> box;
        byteRead += effectiveAtomSize;
        if (QString::fromUtf8(box.type, 4) == path[pathIdx]) {
            return seekTillAtom(reader, path, effectiveAtomSize, pathIdx + 1);
        }
        reader->skip(effectiveAtomSize);
    }
    return 0;
}

} // anonymous namespace

// static
bool StemInfoImporter::hasStemAtom(const QString& filePath) {
    auto file = QFile(filePath);
    if (!file.open(QIODeviceBase::ReadOnly | QIODeviceBase::Unbuffered)) {
        kLogger.warning()
                << "Failed to open input file"
                << filePath;
        return false;
    }
    return seekTillAtom(&file, kStemManifestAtomPath) != 0;
}

// static
bool StemInfoImporter::maybeStemFile(
        const QString& fileName, QMimeType mimeType) {
    if (!mimeType.isValid() || mimeType.isDefault()) {
        // If no MIME type was previously detected for the file content, we read it now.
        mimeType = QMimeDatabase().mimeTypeForFile(
                fileName, QMimeDatabase::MatchContent);
    }
    if (!mimeType.isValid() || mimeType.isDefault()) {
        kLogger.debug() << "Unable to detect MIME type from content in file" << fileName;
        for (const QString& ext : kStemPreferredFileExtensions) {
            if (fileName.endsWith(ext)) {
                return true;
            }
        }
    }
    return kStemMimes.contains(mimeType.name());
}

QList<StemInfo> StemInfoImporter::importStemInfos(
        const QString& filePath) {
    // Fetch the STEM manifest which contain stream details
    auto file = QFile(filePath);
    if (!file.open(QIODeviceBase::ReadOnly | QIODeviceBase::Unbuffered)) {
        kLogger.warning()
                << "Failed to open input file"
                << filePath;
        return {};
    }

    uint32_t manifestSize;
    if (!(manifestSize = seekTillAtom(&file, kStemManifestAtomPath))) {
        kLogger.debug()
                << "No stem manifest found in the file"
                << filePath;
        return {};
    }

    auto jsonData = QJsonDocument::fromJson(file.read(manifestSize));
    VERIFY_OR_DEBUG_ASSERT(jsonData.isObject()) {
        kLogger.warning()
                << "Failed to extract the manifest data"
                << filePath;
        return {};
    };

    auto json = jsonData.object();

    // Check the manifest version
    auto version = json.value("version").toInteger(-1);
    if (version <= 0) {
        kLogger.debug() << "Unexpected or missing version value in STEM manifest";
        return {};
    } else if (version > kSupportedStemVersion) {
        kLogger.warning() << "Unsupported stem version" << version << "but trying anyway";
    }

    // Extract stem metadata
    auto stems = json.value("stems");
    if (!stems.isArray()) {
        kLogger.debug() << "Unexpected or missing stems value in STEM manifest";
        return {};
    }
    const auto stemArray = stems.toArray();
    QList<StemInfo> stemsList;
    stemsList.reserve(stemArray.size());
    int stemIdx = 0;
    for (const auto& stemRef : stemArray) {
        if (!stemRef.isObject()) {
            kLogger.debug() << "Unexpected or missing stems value in STEM manifest";
            return {};
        }
        auto stem = stemRef.toObject();
        auto color = QColor(stem.value("color").toString());
        auto name = stem.value("name").toString();
        if (!color.isValid()) {
            kLogger.debug() << "Unexpected or missing stem color in STEM manifest. Using default";
            color = kStemDefaultColor[stemIdx];
        }
        if (name.isEmpty()) {
            kLogger.debug() << "Unexpected or missing stem name in STEM manifest. Using default";
            name = QObject::tr("Stem #%1").arg(QString::number(stemIdx + 1));
        }
        stemsList.emplace_back(name, color);
        stemIdx++;
    }

    // Extract DSP information
    // TODO(XXX) DSP only contains a limiter and a compressor effect, which
    // Mixxx doesn't have yet. Parse and implement when supported
    auto masteringDsp = json.value("mastering_dsp").toObject();
    auto compressor = masteringDsp.value("compressor").toObject();
    auto limiter = masteringDsp.value("limiter").toObject();

    if (compressor.value("enabled").toBool() || limiter.value("enabled").toBool()) {
        kLogger.debug() << "The STEM manifest is requesting DSP effects but "
                           "this isn't supported by Mixxx yet.";
    }

    file.close();

    return stemsList;
}

} // namespace mixxx
