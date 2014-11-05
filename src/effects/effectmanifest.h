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
class EffectManifest {
  public:
    EffectManifest()
          : m_effectRampsFromDry(false) {
    }

    virtual ~EffectManifest() {
        //qDebug() << debugString() << "deleted";
    }

    virtual const QString& id() const {
        return m_id;
    }
    virtual void setId(const QString& id) {
        m_id = id;
    }

    virtual const QString& name() const {
        return m_name;
    }
    virtual void setName(const QString& name) {
        m_name = name;
    }

    virtual const QString& author() const {
        return m_author;
    }
    virtual void setAuthor(const QString& author) {
        m_author = author;
    }

    virtual const QString& version() const {
        return m_version;
    }
    virtual void setVersion(const QString& version) {
        m_version = version;
    }

    virtual const QString& description() const {
        return m_description;
    }
    virtual void setDescription(const QString& description) {
        m_description = description;
    }

    virtual const QList<EffectManifestParameter>& parameters() const {
        return m_parameters;
    }

    virtual EffectManifestParameter* addParameter() {
        m_parameters.append(EffectManifestParameter());
        return &m_parameters.last();
    }

    virtual bool effectRampsFromDry() const {
        return m_effectRampsFromDry;
    }
    virtual void setEffectRampsFromDry(bool effectFadesFromDry) {
        m_effectRampsFromDry = effectFadesFromDry;
    }

  private:
    QString debugString() const {
        return QString("EffectManifest(%1)").arg(m_id);
    }

    QString m_id;
    QString m_name;
    QString m_author;
    QString m_version;
    QString m_description;
    QList<EffectManifestParameter> m_parameters;
    bool m_effectRampsFromDry;
};

#endif /* EFFECTMANIFEST_H */
