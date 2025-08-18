#include "library/trackset/genre/genrefeaturehelper.h"

#include <QInputDialog>
#include <QLineEdit>

#include "library/trackcollection.h"
#include "library/trackset/genre/genre.h"
#include "library/trackset/genre/genresummary.h"
#include "moc_genrefeaturehelper.cpp"

GenreFeatureHelper::GenreFeatureHelper(
        TrackCollection* pTrackCollection,
        UserSettingsPointer pConfig)
        : m_pTrackCollection(pTrackCollection),
          m_pConfig(pConfig) {
}

QString GenreFeatureHelper::proposeNameForNewGenre(
        const QString& initialName) const {
    DEBUG_ASSERT(!initialName.isEmpty());
    QString proposedName;
    int suffixCounter = 0;
    do {
        if (suffixCounter++ > 0) {
            // Append suffix " 2", " 3", ...
            proposedName = QStringLiteral("%1 %2")
                                   .arg(initialName, QString::number(suffixCounter));
        } else {
            proposedName = initialName;
        }
    } while (m_pTrackCollection->genres().readGenreByName(proposedName));
    // Found an unused genre name
    return proposedName;
}

GenreId GenreFeatureHelper::createEmptyGenre() {
    const QString proposedGenreName =
            proposeNameForNewGenre(tr("New Genre"));
    Genre newGenre;
    for (;;) {
        bool ok = false;
        auto newName =
                QInputDialog::getText(
                        nullptr,
                        tr("Create New Genre"),
                        tr("Enter name for new genre:"),
                        QLineEdit::Normal,
                        proposedGenreName,
                        &ok)
                        .trimmed();
        if (!ok) {
            return GenreId();
        }
        if (newName.isEmpty()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Genre Failed"),
                    tr("A genre cannot have a blank name."));
            continue;
        }
        if (m_pTrackCollection->genres().readGenreByName(newName)) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Genre Failed"),
                    tr("A genre by that name already exists."));
            continue;
        }
        newGenre.setName(std::move(newName));
        DEBUG_ASSERT(newGenre.hasName());
        break;
    }

    GenreId newGenreId;
    if (m_pTrackCollection->insertGenre(newGenre, &newGenreId)) {
        DEBUG_ASSERT(newGenreId.isValid());
        newGenre.setId(newGenreId);
        qDebug() << "Created new genre" << newGenre;
    } else {
        DEBUG_ASSERT(!newGenreId.isValid());
        qWarning() << "Failed to create new genre"
                   << "->" << newGenre.getName();
        QMessageBox::warning(
                nullptr,
                tr("Creating Genre Failed"),
                tr("An unknown error occurred while creating genre: ") + newGenre.getName());
    }
    return newGenreId;
}

GenreId GenreFeatureHelper::duplicateGenre(const Genre& oldGenre) {
    const QString proposedGenreName =
            proposeNameForNewGenre(
                    QStringLiteral("%1 %2")
                            .arg(oldGenre.getName(), tr("copy", "//:")));
    Genre newGenre;
    for (;;) {
        bool ok = false;
        auto newName =
                QInputDialog::getText(
                        nullptr,
                        tr("Duplicate Genre"),
                        tr("Enter name for new genre:"),
                        QLineEdit::Normal,
                        proposedGenreName,
                        &ok)
                        .trimmed();
        if (!ok) {
            return GenreId();
        }
        if (newName.isEmpty()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Duplicating Genre Failed"),
                    tr("A genre cannot have a blank name."));
            continue;
        }
        if (m_pTrackCollection->genres().readGenreByName(newName)) {
            QMessageBox::warning(
                    nullptr,
                    tr("Duplicating Genre Failed"),
                    tr("A genre by that name already exists."));
            continue;
        }
        newGenre.setName(std::move(newName));
        DEBUG_ASSERT(newGenre.hasName());
        break;
    }

    GenreId newGenreId;
    if (m_pTrackCollection->insertGenre(newGenre, &newGenreId)) {
        DEBUG_ASSERT(newGenreId.isValid());
        newGenre.setId(newGenreId);
        qDebug() << "Created new genre" << newGenre;
        QList<TrackId> trackIds;
        trackIds.reserve(
                m_pTrackCollection->genres().countGenreTracks(oldGenre.getId()));
        {
            GenreTrackSelectResult genreTracks(
                    m_pTrackCollection->genres().selectGenreTracksSorted(oldGenre.getId()));
            while (genreTracks.next()) {
                trackIds.append(genreTracks.trackId());
            }
        }
    } else {
        qWarning() << "Failed to duplicate genre"
                   << oldGenre << "->" << newGenre.getName();
        QMessageBox::warning(
                nullptr,
                tr("Duplicating Genre Failed"),
                tr("An unknown error occurred while creating genre: ") + newGenre.getName());
    }
    return newGenreId;
}
