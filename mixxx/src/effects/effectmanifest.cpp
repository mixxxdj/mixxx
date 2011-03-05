#include <QtDebug>

#include "effects/effectmanifest.h"

EffectManifest::EffectManifest(QObject* pParent)
        : QObject() {
}

EffectManifest::~EffectManifest() {
    qDebug() << debugString() << "deleted";
}

const QString EffectManifest::id() const {
    return m_id;
}

void EffectManifest::setId(QString id) {
    m_id = id;
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

const QList<EffectManifestParameterPointer> EffectManifest::parameters() const {
    return m_parameters;
}

void deleteManifestParameter(EffectManifestParameter* pParameter) {
    delete pParameter;
}

EffectManifestParameter* EffectManifest::addParameter() {
    EffectManifestParameter* pParameter = new EffectManifestParameter();

    // We don't use EffectManifestPointer here because that specifies const
    // EffectManifestParameter as the type, which does not work with
    // QObject::deleteLater.
    m_parameters.append(QSharedPointer<EffectManifestParameter>(pParameter, &deleteManifestParameter));
    return pParameter;
}
