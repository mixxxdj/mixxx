#pragma once

#include <QList>
#include <QString>

#include "library/dao/dao.h"
#include "library/dao/genre.h"
#include "util/db/dbid.h"

/// Data Access Object for managing music genres
/// Supports multiple genres per track via normalized tables
class GenreDAO : public DAO {
  public:
    explicit GenreDAO();
    ~GenreDAO() override = default;

    /// Adds a new genre or returns the ID if it already exists
    DbId getOrCreateGenre(const QString& name);

    /// Search for a genre by name (case-insensitive)
    Genre getGenreByName(const QString& name);

    /// Retrieve a genre by ID
    Genre getGenreById(DbId id);

    /// Gets all genres associated with a track
    QList<Genre> getGenresForTrack(DbId trackId);

    /// Set genres for a track (atomic operation)
    /// Removes all existing associations and creates new ones
    bool setGenresForTrack(DbId trackId, const QList<DbId>& genreIds);

    /// Gets all genres present in the database
    QList<Genre> getAllGenres();

    /// Remove unused genres from any track
    int deleteUnusedGenres();
};
