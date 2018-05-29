#ifndef EFFECTMANIFEST_H
#define EFFECTMANIFEST_H

#include <QList>
#include <QString>
#include <QtDebug>

#include "effects/effectmanifestparameter.h"

// An EffectManifest is a full description of the metadata associated with an
// effect (e.g. name, author, version, description, etc.) and the parameters of
// the effect that are intended to be exposed to the rest of Mixxx for user or
// script control.
//
// EffectManifest is composed purely of simple data types, and when an
// EffectManifest is const, it should be completely immutable. EffectManifest is
// meant to be used in most cases as a reference, and in Qt collections, so it
// is important that the implicit copy and assign constructors work, and that
// the no-argument constructor be non-explicit. All methods are left virtual to
// allow a backend to replace the entire functionality with its own (for
// example, a database-backed manifest)
class EffectManifest final {
  public:
    EffectManifest()
        : m_isMixingEQ(false),
          m_isMasterEQ(false),
          m_effectRampsFromDry(false),
          m_metaknobDefault(0.5) {
    }

    const QString& id() const {
        return m_id;
    }
    void setId(const QString& id) {
        m_id = id;
    }

    const QString& name() const {
        return m_name;
    }
    void setName(const QString& name) {
        m_name = name;
    }

    const QString& shortName() const {
        return m_shortName;
    }
    void setShortName(const QString& shortName) {
        m_shortName = shortName;
    }

    const QString& displayName() const {
        if (!m_shortName.isEmpty()) {
            return m_shortName;
        } else {
            return m_name;
        }
    }

    const QString& author() const {
        return m_author;
    }
    void setAuthor(const QString& author) {
        m_author = author;
    }

    const QString& version() const {
        return m_version;
    }
    void setVersion(const QString& version) {
        m_version = version;
    }

    const QString& description() const {
        return m_description;
    }

    const bool& isMixingEQ() const {
        return m_isMixingEQ;
    }

    void setIsMixingEQ(const bool value) {
        m_isMixingEQ = value;
    }

    const bool& isMasterEQ() const {
        return m_isMasterEQ;
    }

    void setIsMasterEQ(const bool value) {
        m_isMasterEQ = value;
    }

    void setDescription(const QString& description) {
        m_description = description;
    }

    const QList<EffectManifestParameter>& parameters() const {
        return m_parameters;
    }

    QList<EffectManifestParameter>& parameters() {
        return m_parameters;
    }

    EffectManifestParameter* addParameter() {
        m_parameters.append(EffectManifestParameter());
        return &m_parameters.last();
    }

    bool effectRampsFromDry() const {
        return m_effectRampsFromDry;
    }
    void setEffectRampsFromDry(bool effectFadesFromDry) {
        m_effectRampsFromDry = effectFadesFromDry;
    }

    double metaknobDefault() const {
        return m_metaknobDefault;
    }
    void setMetaknobDefault(double metaknobDefault) {
        m_metaknobDefault = metaknobDefault;
    }

  private:
    QString debugString() const {
        return QString("EffectManifest(%1)").arg(m_id);
    }

    QString m_id;
    QString m_name;
    QString m_shortName;
    QString m_author;
    QString m_version;
    QString m_description;
    // This helps us at DlgPrefEQ's basic selection of Equalizers
    bool m_isMixingEQ;
    bool m_isMasterEQ;
    QList<EffectManifestParameter> m_parameters;
    bool m_effectRampsFromDry;
    double m_metaknobDefault;
};

#endif /* EFFECTMANIFEST_H */
