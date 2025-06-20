#ifndef GENREDAO_H
#define GENREDAO_H

#include <QList>
#include <QString>

#include "library/dao/dao.h"
#include "library/dao/genre.h"

class DbConnection; // Forward Declaration to reduce compile dependencies

// Data Access Object for all genre-related database operations.
class GenreDAO final : public DAO {
  public:
    explicit GenreDAO(DbConnection* dbConnection);

    // Adds a new genre to the 'genres' table if it doesn't exist.
    // Returns the ID of the new or existing genre. On failure, returns DbId_Invalid.
    DbId addGenre(const QString& name);

    // Retrieves a genre by its name. Returns an invalid Genre if not found.
    Genre getGenreByName(const QString& name);

    // Retrieves a genre by its ID. Returns an invalid Genre if not found.
    Genre getGenreById(DbId id);

    // Retrieves a list of all genres associated with a specific track.
    QList<Genre> getGenresForTrack(DbId trackId);

    // Sets all genres for a specific track, replacing any existing associations.
    bool setGenresForTrack(DbId trackId, const QList<DbId>& genreIds);

    // Retrieves all genres from the database.
    QList<Genre> getAllGenres();

    // Deletes genres from the 'genres' table that are no longer associated with any tracks.
    // Returns the number of rows deleted, or -1 on error.
    int deleteUnusedGenres();
};

#endif
