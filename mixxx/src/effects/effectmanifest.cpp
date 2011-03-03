#include "effects/effectmanifest.h"

EffectManifest::EffectManifest() {
}

EffectManifest::~EffectManifest() {
}

const QString EffectManifest::name() const {
    return m_name;
}

void EffectManifest::setName(QString name) {
    m_name = name;
}

const QString EffectManifest::author() const {
    return m_author;
}

void EffectManifest::setAuthor(QString author) {
    m_author = author;
}

const QString& EffectManifest::version() const {
    return m_version;
}

void EffectManifest::setVersion(QString version) {
    m_version = version;
}

const QString& EffectManifest::description() const {
    return m_description;
}

void EffectManifest::setDescription(QString description) {
    m_description = description;
}

const QList<EffectManifestParameter> EffectManifest::parameters() const {
    return m_parameters;
}

EffectManifestParameter& EffectManifest::addParameter() {
    m_parameters.append(EffectManifestParameter());
    return m_parameters.last();
}
