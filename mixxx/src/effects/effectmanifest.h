#ifndef EFFECTMANIFEST_H
#define EFFECTMANIFEST_H

#include <QObject>
#include <QList>
#include <QString>
#include <QSharedPointer>

#include "effects/effectmanifestparameter.h"

class EffectManifest;
typedef QSharedPointer<const EffectManifest> EffectManifestPointer;

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
class EffectManifest : public QObject {
    Q_OBJECT
  public:
    EffectManifest(QObject* pParent = NULL);
    virtual ~EffectManifest();

    virtual const QString id() const;
    virtual void setId(QString id);

    virtual const QString name() const;
    virtual void setName(QString name);

    virtual const QString author() const;
    virtual void setAuthor(QString author);

    virtual const QString& version() const;
    virtual void setVersion(QString version);

    virtual const QString& description() const;
    virtual void setDescription(QString description);

    virtual const QList<EffectManifestParameterPointer> parameters() const;
    virtual EffectManifestParameter* addParameter();

  private:
    QString debugString() const {
        return QString("EffectManifest(%1)").arg(m_id);
    }

    QString m_id;
    QString m_name;
    QString m_author;
    QString m_version;
    QString m_description;
    QList<EffectManifestParameterPointer> m_parameters;
};

#endif /* EFFECTMANIFEST_H */
