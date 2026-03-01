#include "track/steminfo.h"

#include <QDebugStateSaver>

#include "engine/engine.h"
#include "util/assert.h"

namespace mixxx {

const QColor StemInfo::kStemDefaultColor[4] = {
        QColor(0x00, 0x9E, 0x73),
        QColor(0xD5, 0x5E, 0x00),
        QColor(0xCC, 0x79, 0xA7),
        QColor(0x56, 0xB4, 0xE9),
};

const QString StemInfo::kStemDefaultLabel[4] = {
        QStringLiteral("Drums"),
        QStringLiteral("Bass"),
        QStringLiteral("Others"),
        QStringLiteral("Vocals"),
};

QByteArray StemInfo::toByteArray() const {
    track::io::StemInfo info;
    info.set_version(version());
    for (auto it = begin(); it != end(); it++) {
        track::io::Stem stem;
        google::protobuf::int32 red = static_cast<google::protobuf::int32>(
                                        it->getColor().redF() * 255.f),
                                green = static_cast<google::protobuf::int32>(
                                        it->getColor().greenF() * 255.f),
                                blue = static_cast<google::protobuf::int32>(
                                        it->getColor().blueF() * 255.f);
        stem.set_color(red << 16 | green << 8 | blue);
        stem.set_label(it->getLabel().toStdString());
        info.add_stems()->CopyFrom(stem);
    }
    info.mutable_masteringdsp()->CopyFrom(masteringDSP().toProto());

    std::string output;
    info.SerializeToString(&output);
    return QByteArray(output.data(), static_cast<int>(output.length()));
}

StemInfo StemInfo::fromByteArray(
        const QByteArray& stemInfoSerialized) {
    track::io::StemInfo info;
    if (!info.ParseFromArray(stemInfoSerialized.constData(), stemInfoSerialized.size())) {
        return {};
    }

    StemInfo stemInfo(info.version());
    stemInfo.reserve(info.stems_size());

    for (int i = 0; i < info.stems_size(); i++) {
        auto color = info.stems(i).color();
        stemInfo.emplace_back(QString::fromStdString(info.stems(i).label()),
                QColor(color >> 16, color >> 8 & 0xff, color & 0xff));
    }
    stemInfo.setMasteringDSP(MasteringDSP::fromProto(info.masteringdsp()));

    VERIFY_OR_DEBUG_ASSERT(stemInfo.isValid()) {
        qWarning() << "Failed to deserialize StemInfo: StemInfo contains only"
                   << stemInfo.size() << "stem(s)," << mixxx::kMaxSupportedStems << "are needed";
        return {};
    }

    return stemInfo;
}

StemInfo StemInfo::newDefault() {
    StemInfo stemInfo(kSupportedVersion);
    stemInfo.reserve(kMaxSupportedStems);

    for (int stemIdx = 0; stemIdx < kMaxSupportedStems; stemIdx++) {
        stemInfo.emplace_back(
                kStemDefaultLabel[stemIdx],
                kStemDefaultColor[stemIdx]);
    }
    stemInfo.setMasteringDSP({{
                                      false,
                                      10,
                                      0,
                                      1,
                                      0.0001,
                                      0,
                                      0,
                                      20,
                                      100,
                              },
            {
                    false,
                    1,
                    0,
                    0,
            }});

    DEBUG_ASSERT(stemInfo.isValid());

    return stemInfo;
}

StemInfo::MasteringDSP StemInfo::MasteringDSP::fromJson(const QJsonObject& data) {
    auto compressorData = data.value("compressor").toObject();
    auto limiterData = data.value("limiter").toObject();

    return {
            {
                    compressorData.value("enabled").toBool(),
                    compressorData.value("ratio").toInt(10),
                    compressorData.value("output_gain").toInt(),
                    compressorData.value("release").toDouble(1),
                    compressorData.value("attack").toDouble(0.0001),
                    compressorData.value("input_gain").toInt(),
                    compressorData.value("threshold").toInt(),
                    compressorData.value("hp_cutoff").toInt(20),
                    compressorData.value("dry_wet").toInt(100),
            },
            {
                    limiterData.value("enabled").toBool(),
                    limiterData.value("release").toDouble(1),
                    limiterData.value("threshold").toInt(),
                    limiterData.value("ceiling").toInt(),
            }};
}

StemInfo::MasteringDSP StemInfo::MasteringDSP::fromProto(const track::io::MasteringDSP& data) {
    return {
            {
                    data.compressor().enabled(),
                    data.compressor().ratio(),
                    data.compressor().output_gain(),
                    data.compressor().release(),
                    data.compressor().attack(),
                    data.compressor().input_gain(),
                    data.compressor().threshold(),
                    data.compressor().hp_cutoff(),
                    data.compressor().dry_wet(),
            },
            {
                    data.limiter().enabled(),
                    data.limiter().release(),
                    data.limiter().threshold(),
                    data.limiter().ceiling(),
            }};
}

QJsonObject StemInfo::MasteringDSP::toJson() const {
    return {
            {"compressor", QJsonObject({
                                   {"enabled", compressor.enabled},
                                   {"ratio", compressor.ratio},
                                   {"output_gain", compressor.output_gain},
                                   {"release", compressor.release},
                                   {"attack", compressor.attack},
                                   {"input_gain", compressor.input_gain},
                                   {"threshold", compressor.threshold},
                                   {"hp_cutoff", compressor.hp_cutoff},
                                   {"dry_wet", compressor.dry_wet},
                           })},
            {"limiter", QJsonObject({
                                {"enabled", limiter.enabled},
                                {"release", limiter.release},
                                {"threshold", limiter.threshold},
                                {"ceiling", limiter.ceiling},
                        })},
    };
}

track::io::MasteringDSP StemInfo::MasteringDSP::toProto() const {
    track::io::MasteringDSP proto;

    proto.mutable_compressor()->set_enabled(compressor.enabled);
    proto.mutable_compressor()->set_ratio(compressor.ratio);
    proto.mutable_compressor()->set_output_gain(compressor.output_gain);
    proto.mutable_compressor()->set_release(compressor.release);
    proto.mutable_compressor()->set_attack(compressor.attack);
    proto.mutable_compressor()->set_input_gain(compressor.input_gain);
    proto.mutable_compressor()->set_threshold(compressor.threshold);
    proto.mutable_compressor()->set_hp_cutoff(compressor.hp_cutoff);
    proto.mutable_compressor()->set_dry_wet(compressor.dry_wet);

    proto.mutable_limiter()->set_enabled(limiter.enabled);
    proto.mutable_limiter()->set_release(limiter.release);
    proto.mutable_limiter()->set_threshold(limiter.threshold);
    proto.mutable_limiter()->set_ceiling(limiter.ceiling);

    return proto;
}

QDebug operator<<(QDebug dbg, const StemInfo& stemInfo) {
    const QDebugStateSaver saver(dbg);
    dbg = dbg.maybeSpace() << "StemInfo";
    dbg = dbg.nospace() << '{';
    for (const auto& stem : stemInfo) {
        dbg = dbg << stem << ',';
    }
    return dbg << '}';
}

QDebug operator<<(QDebug dbg, const Stem& stemInfo) {
    const QDebugStateSaver saver(dbg);
    dbg = dbg.maybeSpace() << "Stem";
    return dbg.nospace()
            << '{'
            << stemInfo.getLabel()
            << ','
            << stemInfo.getColor().name()
            << '}';
}

bool operator==(
        const Stem& lhs,
        const Stem& rhs) {
    return lhs.getLabel() == rhs.getLabel() && lhs.getColor() == rhs.getColor();
}
} // namespace mixxx
