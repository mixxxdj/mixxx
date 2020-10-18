#include "widget/wsearchrelatedtracksmenu.h"

#include "track/track.h"
#include "util/qt.h"

WSearchRelatedTracksMenu::WSearchRelatedTracksMenu(
        QWidget* parent)
        : QMenu(tr("Search related Tracks"), parent) {
}

void WSearchRelatedTracksMenu::addActionsForTrack(
        const Track& track) {
    bool addSeparator = false;

    // Musical property actions
    const auto keyText = track.getKeyText();
    if (!keyText.isEmpty()) {
        const auto keyTextDisplay =
                mixxx::escapeTextPropertyWithoutShortcuts(keyText);
        addSeparator = addSeparatorBeforeAction(addSeparator);
        addAction(
                tr("Harmonic keys for \"%1\"")
                        .arg(keyTextDisplay),
                [this, keyText]() {
                    emit triggerSearch(
                            QStringLiteral("~key:\"") +
                            keyText + QChar('"'));
                });
    }
    const auto genre = track.getGenre();
    if (!genre.isEmpty()) {
        const auto genreDisplay =
                mixxx::escapeTextPropertyWithoutShortcuts(genre);
        addSeparator = addSeparatorBeforeAction(addSeparator);
        addAction(
                tr("Genre \"%1\"")
                        .arg(genreDisplay),
                [this, genre]() {
                    emit triggerSearch(
                            QStringLiteral("genre:\"") +
                            genre + QChar('"'));
                });
    }

    // Artist actions
    addSeparator = true;
    auto primaryArtist = track.getArtist();
    auto secondaryArtist = track.getAlbumArtist();
    if (primaryArtist.isEmpty()) {
        primaryArtist = secondaryArtist;
        secondaryArtist = QString();
    } else {
        if (!secondaryArtist.isEmpty() &&
                primaryArtist.contains(secondaryArtist)) {
            // Use the shorter string as primary artist and the
            // longer string as secondary artist
            if (primaryArtist == secondaryArtist) {
                secondaryArtist = QString();
            } else {
                std::swap(primaryArtist, secondaryArtist);
            }
        }
    }
    DEBUG_ASSERT(!primaryArtist.isEmpty() || secondaryArtist.isEmpty());
    if (!primaryArtist.isEmpty()) {
        const auto primaryArtistDisplay =
                mixxx::escapeTextPropertyWithoutShortcuts(primaryArtist);
        const auto secondaryArtistDisplay =
                mixxx::escapeTextPropertyWithoutShortcuts(secondaryArtist);
        // Search tracks with similar artist(s)
        addSeparator = addSeparatorBeforeAction(addSeparator);
        addAction(
                tr("Artist \"%1\"")
                        .arg(primaryArtistDisplay),
                [this, primaryArtist]() {
                    emit triggerSearch(
                            QStringLiteral("artist:\"") +
                            primaryArtist + QChar('"'));
                });
        if (!secondaryArtist.isEmpty()) {
            addSeparator = addSeparatorBeforeAction(addSeparator);
            addAction(
                    tr("Artist \"%1\"")
                            .arg(secondaryArtistDisplay),
                    [this, secondaryArtist]() {
                        emit triggerSearch(
                                QStringLiteral("artist:\"") +
                                secondaryArtist + QChar('"'));
                    });
        }
        addSeparator = addSeparatorBeforeAction(addSeparator);
        addAction(
                tr("Album artist \"%1\"")
                        .arg(primaryArtistDisplay),
                [this, primaryArtist]() {
                    emit triggerSearch(
                            QStringLiteral("album_artist:\"") +
                            primaryArtist + QChar('"'));
                });
        if (!secondaryArtist.isEmpty()) {
            addSeparator = addSeparatorBeforeAction(addSeparator);
            addAction(
                    tr("Album artist \"%1\"")
                            .arg(secondaryArtistDisplay),
                    [this, secondaryArtist]() {
                        emit triggerSearch(
                                QStringLiteral(
                                        "album_artist:\"") +
                                secondaryArtist + QChar('"'));
                    });
        }
    }
    const auto composer = track.getComposer();
    if (!composer.isEmpty()) {
        const auto composerDisplay =
                mixxx::escapeTextPropertyWithoutShortcuts(composer);
        addSeparator = addSeparatorBeforeAction(addSeparator);
        addAction(
                tr("Composer \"%1\"")
                        .arg(composerDisplay),
                [this, composer]() {
                    emit triggerSearch(
                            QStringLiteral("composer:\"") +
                            composer + QChar('"'));
                });
    }

    // Title actions
    addSeparator = true;
    const auto title = track.getTitle();
    if (!title.isEmpty()) {
        const auto titleDisplay =
                mixxx::escapeTextPropertyWithoutShortcuts(title);
        addSeparator = addSeparatorBeforeAction(addSeparator);
        addAction(
                tr("Title \"%1\"")
                        .arg(titleDisplay),
                [this, title]() {
                    emit triggerSearch(
                            QStringLiteral("title:\"") +
                            title + QChar('"'));
                });
    }
    const auto album = track.getAlbum();
    if (!album.isEmpty()) {
        const auto albumDisplay =
                mixxx::escapeTextPropertyWithoutShortcuts(album);
        addSeparator = addSeparatorBeforeAction(addSeparator);
        addAction(
                tr("Album \"%1\"")
                        .arg(albumDisplay),
                [this, album]() {
                    emit triggerSearch(
                            QStringLiteral("album:\"") +
                            album + QChar('"'));
                });
    }

    // File system actions
    addSeparator = true;
    const auto locationPath = track.getFileInfo().directory();
    if (!locationPath.isEmpty()) {
        const auto locationPathDisplay =
                mixxx::escapeTextPropertyWithoutShortcuts(locationPath);
        if (addSeparator) {
            this->addSeparator();
            addSeparator = false;
        }
        addSeparator = addSeparatorBeforeAction(addSeparator);
        addAction(
                tr("Folder \"%1\"")
                        .arg(locationPathDisplay),
                [this, locationPath]() {
                    emit triggerSearch(
                            QStringLiteral("location:\"") +
                            locationPath + QChar('"'));
                });
    }
}
