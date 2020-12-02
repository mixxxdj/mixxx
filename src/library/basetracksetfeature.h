#pragma once

#include <QByteArrayData>
#include <QString>

#include "library/libraryfeature.h"
#include "preferences/usersettings.h"

class Library;
class QObject;
class TrackId;
template<class T>
class QList;

class BaseTrackSetFeature : public LibraryFeature {
    Q_OBJECT
  public:
    BaseTrackSetFeature(Library* pLibrary,
            UserSettingsPointer pConfig,
            const QString& rootViewName);

  signals:
    void analyzeTracks(const QList<TrackId>&);

  public slots:
    void activate() override;

  protected:
    const QString m_rootViewName;
};
