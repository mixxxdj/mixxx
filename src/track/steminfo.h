#pragma once

#include <QColor>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>
#include <algorithm>

#include "engine/engine.h"
#include "proto/stems.pb.h"

namespace mixxx {
class Stem {
  public:
    Stem(const QString& label = QString(), const QColor& color = QColor())
            : m_label(label),
              m_color(color) {
    }

    const QString& getLabel() const {
        return m_label;
    }
    void setLabel(const QString& label) {
        m_label = label;
    }

    const QColor& getColor() const {
        return m_color;
    }
    void setColor(const QColor& color) {
        m_color = color;
    }

    bool isValid() const {
        return !m_label.isEmpty() && m_color.isValid();
    }

  private:
    QString m_label;
    QColor m_color;
};

class StemInfo : private QList<Stem> {
  public:
    using QList::at;
    using QList::begin;
    using QList::end;
    using QList::size;
    using QList::operator[];
    using QList::operator==;
    using QList::operator!=;

    struct Compressor {
        bool enabled;
        int ratio;
        int output_gain;
        double release;
        double attack;
        int input_gain;
        int threshold;
        int hp_cutoff;
        int dry_wet;
    };

    struct Limiter {
        bool enabled;
        double release;
        int threshold;
        int ceiling;
    };

    struct MasteringDSP {
        Compressor compressor;
        Limiter limiter;

        static MasteringDSP fromJson(const QJsonObject&);
        static MasteringDSP fromProto(const track::io::MasteringDSP&);
        QJsonObject toJson() const;
        track::io::MasteringDSP toProto() const;
    };

    StemInfo(int version = kSupportedVersion)
            : m_version(version) {};
    StemInfo(const Stem& first,
            const Stem& second,
            const Stem& third,
            const Stem& fourth,
            int version = kSupportedVersion)
            : QList<Stem>({first, second, third, fourth}),
              m_masteringDsp({}),
              m_version(version) {
    }

    static StemInfo fromByteArray(
            const QByteArray& stemInfoSerialized);
    static StemInfo newDefault();

    QByteArray toByteArray() const;

    bool isValid() const {
        return size() == mixxx::kMaxSupportedStems &&
                m_version == kSupportedVersion &&
                std::find_if_not(cbegin(), cend(), [](const auto& it) {
                    return it.isValid();
                }) == cend();
    }

    const MasteringDSP& masteringDSP() const {
        return m_masteringDsp;
    }

    void setMasteringDSP(const MasteringDSP& masteringDsp) {
        m_masteringDsp = masteringDsp;
    }

    int version() const {
        return m_version;
    }

    static constexpr int kSupportedVersion = 1;
    static const QColor kStemDefaultColor[kMaxSupportedStems];
    static const QString kStemDefaultLabel[kMaxSupportedStems];

  private:
    const QList<Stem>& upcast() const {
        return *this;
    }

    friend bool operator==(
            const StemInfo& lhs,
            const StemInfo& rhs);

    MasteringDSP m_masteringDsp;
    int m_version;

    friend class StemInfoImporter;
};

bool operator==(
        const Stem& lhs,
        const Stem& rhs);

inline bool operator!=(
        const Stem& lhs,
        const Stem& rhs) {
    return !(lhs == rhs);
}

QDebug operator<<(QDebug debug, const Stem& stemInfo);

inline bool operator==(
        const StemInfo& lhs,
        const StemInfo& rhs) {
    return static_cast<QList<Stem>>(lhs) == static_cast<QList<Stem>>(rhs);
}

inline bool operator!=(
        const StemInfo& lhs,
        const StemInfo& rhs) {
    return !(lhs == rhs);
}

QDebug operator<<(QDebug debug, const StemInfo& stemInfo);

StemInfo::MasteringDSP operator<<(StemInfo::MasteringDSP dsp,
        const QJsonObject& data);
} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::Stem);
Q_DECLARE_METATYPE(mixxx::StemInfo);
