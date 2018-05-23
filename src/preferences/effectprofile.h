#ifndef EFFECTPROFILE_H
#define EFFECTPROFILE_H

#include <QSharedPointer>
#include <QObject>
#include <QString>

#include "effects/defs.h"
#include "effects/effectmanifest.h"
#include "preferences/usersettings.h"

class EffectProfile;
typedef QSharedPointer<EffectProfile> EffectProfilePtr;
Q_DECLARE_METATYPE(EffectProfilePtr)

class EffectProfile : public QObject {
  Q_OBJECT

  public:
    EffectProfile(EffectManifestPointer pManifest,
                  bool visibility,
                  QObject* parent = NULL);

    QString getEffectId() const;
    QString getDisplayName() const;
    bool isVisible() const;
    void setVisibility(bool value);
    EffectManifestPointer getManifest() const;

  private:
    EffectManifestPointer m_pManifest;
    bool m_isVisible;
};

#endif // EFFECTPROFILE_H
