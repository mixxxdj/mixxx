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

#include "sources/soundsourcestem.h"
#include "track/steminfo.h"
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
        const QString& fileName, QMimeType mimeType, bool checkFileContent) {
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
    bool result = kStemMimes.contains(mimeType.name());
    if (!result || !checkFileContent) {
        return result;
    }

    mixxx::AudioSource::OpenParams config;
    config.setChannelCount(mixxx::audio::ChannelCount(8));
    return SoundSourceSTEM(QUrl::fromLocalFile(fileName))
                   .open(AudioSource::OpenMode::Strict, config) ==
            AudioSource::OpenResult::Succeeded;
}

StemInfo StemInfoImporter::importStemInfos(
        const QByteArray& manifest) {
    // Fetch the STEM manifest which contain stream details
    auto jsonData = QJsonDocument::fromJson(manifest);
    VERIFY_OR_DEBUG_ASSERT(jsonData.isObject()) {
        kLogger.warning()
                << "Failed to extract the manifest data";
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
    StemInfo stemsInfo(version);
    stemsInfo.reserve(stemArray.size());
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
            color = StemInfo::kStemDefaultColor[stemIdx];
        }
        if (name.isEmpty()) {
            kLogger.debug() << "Unexpected or missing stem name in STEM manifest. Using default";
            name = QObject::tr("Stem #%1").arg(QString::number(stemIdx + 1));
        }
        stemsInfo.emplace_back(name, color);
        stemIdx++;
    }

    // Extract DSP information
    // TODO(XXX) DSP only contains a limiter and a compressor effect, which
    // Mixxx doesn't have yet. Parse and implement when supported
    auto masteringDspData = json.value("mastering_dsp").toObject();

    auto masteringDsp = StemInfo::MasteringDSP::fromJson(masteringDspData);

    if (masteringDsp.compressor.enabled || masteringDsp.limiter.enabled) {
        kLogger.debug() << "The STEM manifest is requesting DSP effects but "
                           "this isn't supported by Mixxx yet.";
    }
    stemsInfo.setMasteringDSP(masteringDsp);

    return stemsInfo;
}

QByteArray StemInfoImporter::exportStemInfos(
        const StemInfo& manifest) {
    VERIFY_OR_DEBUG_ASSERT(manifest.isValid()) {
        kLogger.warning()
                << "Failed to serialise the manifest data";
        return {};
    };

    QJsonObject jsonData{{{"version", manifest.version()}}};

    // Extract stem metadata
    QJsonArray stemList;
    for (const auto& stem : manifest) {
        stemList.append(QJsonObject({
                {"color", stem.getColor().name()},
                {"name", stem.getLabel()},
        }));
    }
    jsonData.insert("stems", stemList);
    jsonData.insert("mastering_dsp", manifest.masteringDSP().toJson());

    return QJsonDocument(jsonData).toJson(QJsonDocument::JsonFormat::Compact);
}

} // namespace mixxx
