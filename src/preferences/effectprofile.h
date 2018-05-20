#ifndef EFFECTPROFILE_H
#define EFFECTPROFILE_H

#include <QSharedPointer>
#include <QObject>
#include <QString>

#include "preferences/usersettings.h"
#include "effects/effectmanifest.h"

class EffectProfile;
typedef QSharedPointer<EffectProfile> EffectProfilePtr;
Q_DECLARE_METATYPE(EffectProfilePtr)

class EffectProfile : public QObject {
  Q_OBJECT

  public:
    EffectProfile(EffectManifest &pManifest,
                                 QObject* parent = NULL);

    QString getEffectId() const;
    QString getDisplayName() const;
    bool isVisible() const;
    void setVisibility(bool value);
    EffectManifest* getManifest() const;

  private:
    bool m_isVisible;
    EffectManifest* m_pManifest;
};

#endif // EFFECTPROFILE_H
