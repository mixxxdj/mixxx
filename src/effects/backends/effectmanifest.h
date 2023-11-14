#pragma once

#include <QList>
#include <QSharedPointer>
#include <QString>

#include "effects/backends/effectmanifestparameter.h"
#include "effects/backends/effectsbackend.h"
#include "effects/defs.h"

/// An EffectManifest is a description of the metadata associated with an effect
/// (ID, display name, author, description) and all the parameters of the effect.
/// The pair of the ID string and EffectBackendType uniquely identifies an
/// effect. EffectManifests are used by EffectBackends to create EffectProcessors
/// which implement the DSP logic of the effect. The name string of effect
/// parameters in the manifest is used to link EngineEffectParameters
/// with member variables used in the DSP logic of the EffectProcessorImpl.
///
/// EffectManifest is composed purely of simple data types, and when an
/// EffectManifest is const, it should be completely immutable. EffectManifest is
/// meant to be used in most cases as a reference, and in Qt collections, so it
/// is important that the implicit copy and assign constructors work, and that
/// the no-argument constructor be non-explicit.
class EffectManifest {
  public:
    EffectManifest()
            : m_backendType(EffectBackendType::Unknown),
              m_isMixingEQ(false),
              m_isMainEQ(false),
              m_effectRampsFromDry(false),
              m_bAddDryToWet(false),
              m_metaknobDefault(0.0) {
    }

    /// Hack to store unique IDs in QComboBox models
    const QString uniqueId() const {
        return m_id + " " + EffectsBackend::backendTypeToString(m_backendType);
    }

    /// WARNING! Effects must not be identified solely by ID string or name.
    /// ID strings and names are only unique among EffectManifests from one
    /// EffectsBackend. Use EffectManifest::operator== to compare both ID string
    /// and EffectBackendType.
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

    const EffectBackendType& backendType() const {
        return m_backendType;
    }
    void setBackendType(const EffectBackendType& type) {
        m_backendType = type;
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

    const bool& isMainEQ() const {
        return m_isMainEQ;
    }

    void setIsMainEQ(const bool value) {
        m_isMainEQ = value;
    }

    bool hasMetaKnobLinking() const;

    void setDescription(const QString& description) {
        m_description = description;
    }

    const QList<EffectManifestParameterPointer>& parameters() const {
        return m_parameters;
    }

    EffectManifestParameterPointer addParameter() {
        EffectManifestParameterPointer effectManifestParameterPointer(
                new EffectManifestParameter());
        effectManifestParameterPointer->setIndex(m_parameters.size());
        m_parameters.append(effectManifestParameterPointer);
        return effectManifestParameterPointer;
    }

    EffectManifestParameterPointer parameter(int i) {
        return m_parameters[i];
    }

    bool effectRampsFromDry() const {
        return m_effectRampsFromDry;
    }
    void setEffectRampsFromDry(bool effectFadesFromDry) {
        m_effectRampsFromDry = effectFadesFromDry;
    }

    bool addDryToWet() const {
        return m_bAddDryToWet;
    }
    void setAddDryToWet(bool addDryToWet) {
        m_bAddDryToWet = addDryToWet;
    }

    double metaknobDefault() const {
        return m_metaknobDefault;
    }
    void setMetaknobDefault(double metaknobDefault) {
        m_metaknobDefault = metaknobDefault;
    }

    bool operator==(const EffectManifest& other) const {
        return other.id() == m_id && other.backendType() == m_backendType;
    }

    bool operator<(const EffectManifest& other) const {
        if (other.backendType() != m_backendType) {
            return other.backendType() < m_backendType;
        }
        return other.id() < m_id;
    }

    static bool sortLexigraphically(
            EffectManifestPointer pManifest1, EffectManifestPointer pManifest2);

  private:
    QString debugString() const {
        return QString("EffectManifest(%1)").arg(m_id);
    }

    QString m_id;
    QString m_name;
    QString m_shortName;
    EffectBackendType m_backendType;
    QString m_author;
    QString m_version;
    QString m_description;
    /// This helps us at DlgPrefEQ's basic selection of Equalizers
    bool m_isMixingEQ;
    bool m_isMainEQ;
    QList<EffectManifestParameterPointer> m_parameters;
    bool m_effectRampsFromDry;
    bool m_bAddDryToWet;
    double m_metaknobDefault;
};
