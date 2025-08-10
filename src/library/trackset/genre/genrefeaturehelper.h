#pragma once

#include <QObject>

#include "library/trackset/genre/genreid.h"
#include "preferences/usersettings.h"

class TrackCollection;
class Genre;

class GenreFeatureHelper : public QObject {
    Q_OBJECT

  public:
    GenreFeatureHelper(
            TrackCollection* pTrackCollection,
            UserSettingsPointer pConfig);
    ~GenreFeatureHelper() override = default;

    GenreId createEmptyGenre();
    GenreId duplicateGenre(const Genre& oldGenre);

  private:
    QString proposeNameForNewGenre(
            const QString& initialName = QString()) const;

    TrackCollection* m_pTrackCollection;

    UserSettingsPointer m_pConfig;
};
