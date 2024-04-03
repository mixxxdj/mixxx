#include "track/steminfoimporter.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include "util/logger.h"

#define ATOM_TYPE(value)                                                   \
    (uint32_t)(((uint8_t)(value)[0] << 24) | ((uint8_t)(value)[1] << 16) | \
            ((uint8_t)(value)[2] << 8) | (uint8_t)(value)[3])

namespace mixxx {

namespace {

const mixxx::Logger kLogger("StemInfoImporter");
constexpr int kSupportedStemVersion = 1;
const QString kStemFileExtension = QStringLiteral(".stem.mp4");

const uint32_t kAtomHeaderSize = 8; // 4 bytes (unsigned integer) + 4 char
const uint32_t kStemManifestAtomPath[] = {
        ATOM_TYPE("moov"),
        ATOM_TYPE("udta"),
        ATOM_TYPE("stem"),
        0 // This indicate end of the path
};

/// @brief Seek the reader the atom requested.
/// @param reader The IODevice to search MP4 atom in
/// @param path The list of atom to traverse in the tree, from top to bottom.
/// Must be null terminated
/// @param box_size The size of the box currently under the cursor to focus the
/// search in
/// @param pathIdx The level of the tree currently search (index of path)
/// @return the size of the box data if found, -1 otherwise
uint32_t seekTillAtom(QIODevice& reader,
        const uint32_t path[],
        uint32_t box_size = 0,
        uint32_t pathIdx = 0) {
    if (!path[pathIdx]) {
        return box_size;
    }

    uint32_t byteRead = 0;
    char buffer[kAtomHeaderSize];
    while (!reader.atEnd() || (box_size && byteRead >= box_size)) {
        reader.read(buffer, kAtomHeaderSize);
        byteRead += box_size;
        if (ATOM_TYPE(buffer + 4) == path[pathIdx]) {
            return seekTillAtom(reader, path, ATOM_TYPE(buffer) - kAtomHeaderSize, pathIdx + 1);
        }
        reader.skip(ATOM_TYPE(buffer) - kAtomHeaderSize);
    }
    return -1;
}

} // anonymous namespace

// static
bool StemInfoImporter::isStemFile(
        const QString& aFileName) {
    return aFileName.endsWith(kStemFileExtension);
}

QList<StemInfo> StemInfoImporter::importStemInfos(
        const QString& filePath) {
    // Fetch the STEM manifest which contain stream details
    auto file = QFile(filePath);
    if (!file.open(QIODeviceBase::ReadOnly)) {
        kLogger.warning()
                << "Failed to open input file"
                << filePath;
        return {};
    }

    uint32_t manifestSize;
    if (!(manifestSize = seekTillAtom(file, kStemManifestAtomPath))) {
        kLogger.warning()
                << "Failed to find the steam manifest in the file"
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
        kLogger.debug() << "Unsupported stem version" << version << "but trying anyway";
    }

    // Extract stem metadata
    auto stems = json.value("stems");
    if (!stems.isArray()) {
        kLogger.debug() << "Unexpected or missing stems value in STEM manifest";
        return {};
    }
    auto stemArray = stems.toArray();
    QList<StemInfo> stemsList;
    stemsList.reserve(stemArray.size());
    for (const auto& stemRef : stemArray) {
        if (!stemRef.isObject()) {
            kLogger.debug() << "Unexpected or missing stems value in STEM manifest";
            return {};
        }
        auto stem = stemRef.toObject();
        auto color = QColor(stem.value("color").toString());
        auto name = stem.value("name").toString();
        if (!color.isValid() || name.isEmpty()) {
            kLogger.debug() << "Unexpected or missing stem name or attribute in STEM manifest";
            return {};
        }
        stemsList.emplace_back(name, color);
    }

    // Extract DSP information
    // TODO(XXX) DSP only contains limit and a compressor, which aren't
    // supported by Mixxx yet. parse and implement when supported

    file.close();

    return stemsList;
}

} // namespace mixxx
