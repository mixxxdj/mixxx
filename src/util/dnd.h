#ifndef DND_H
#define DND_H

#include <QUrl>
#include <QDrag>
#include <QMimeData>
#include <QList>
#include <QString>
#include <QFileInfo>
#include <QRegExp>
#include <QScopedPointer>

#include "configobject.h"
#include "controlobject.h"
#include "soundsourceproxy.h"
#include "library/parser.h"
#include "library/parserm3u.h"
#include "library/parserpls.h"
#include "library/parsercsv.h"
#include "util/sandbox.h"
#include "playermanager.h"

class DragAndDropHelper {
  public:
    static QList<QFileInfo> supportedTracksFromUrls(const QList<QUrl>& urls,
                                                    bool firstOnly,
                                                    bool acceptPlaylists) {
        QList<QFileInfo> fileLocations;
        foreach (const QUrl& url, urls) {

            // XXX: Possible WTF alert - Previously we thought we needed
            // toString() here but what you actually want in any case when
            // converting a QUrl to a file system path is
            // QUrl::toLocalFile(). This is the second time we have flip-flopped
            // on this, but I think toLocalFile() should work in any
            // case. toString() absolutely does not work when you pass the
            // result to a (this comment was never finished by the original
            // author).
            QString file(url.toLocalFile());

            // If the file is on a network share, try just converting the URL to
            // a string...
            if (file.isEmpty()) {
                file = url.toString();
            }

            if (file.isEmpty()) {
                continue;
            }

            if (acceptPlaylists && (file.endsWith(".m3u") || file.endsWith(".m3u8"))) {
                QScopedPointer<ParserM3u> playlist_parser(new ParserM3u());
                QList<QString> track_list = playlist_parser->parse(file);
                foreach (const QString& playlistFile, track_list) {
                    addFileToList(playlistFile, &fileLocations);
                }
            } else if (acceptPlaylists && url.toString().endsWith(".pls")) {
                QScopedPointer<ParserPls> playlist_parser(new ParserPls());
                QList<QString> track_list = playlist_parser->parse(file);
                foreach (const QString& playlistFile, track_list) {
                    addFileToList(playlistFile, &fileLocations);
                }
            } else {
                addFileToList(file, &fileLocations);
            }

            if (firstOnly && !fileLocations.isEmpty()) {
                break;
            }
        }

        return fileLocations;
    }

    // Allow loading to a player if the player isn't playing or the settings
    // allow interrupting a playing player.
    static bool allowLoadToPlayer(const QString& group,
                                  ConfigObject<ConfigValue>* pConfig) {
        return allowLoadToPlayer(
                group, ControlObject::get(ConfigKey(group, "play")) > 0.0,
                pConfig);
    }

    // Allow loading to a player if the player isn't playing or the settings
    // allow interrupting a playing player.
    static bool allowLoadToPlayer(const QString& group,
                                  bool isPlaying,
                                  ConfigObject<ConfigValue>* pConfig) {
        // Always allow loads to preview decks.
        if (PlayerManager::isPreviewDeckGroup(group)) {
            return true;
        }

        return !isPlaying || pConfig->getValueString(
                ConfigKey("[Controls]",
                          "AllowTrackLoadToPlayingDeck")).toInt();
    }

    static bool dragEnterAccept(const QMimeData& mimeData,
                                const QString& sourceIdentifier,
                                bool firstOnly,
                                bool acceptPlaylists) {
        QList<QFileInfo> files = dropEventFiles(mimeData, sourceIdentifier,
                                                firstOnly, acceptPlaylists);
        return !files.isEmpty();
    }

    static QList<QFileInfo> dropEventFiles(const QMimeData& mimeData,
                                           const QString& sourceIdentifier,
                                           bool firstOnly,
                                           bool acceptPlaylists) {
        if (!mimeData.hasUrls() ||
                (mimeData.hasText() && mimeData.text() == sourceIdentifier)) {
            return QList<QFileInfo>();
        }

        QList<QFileInfo> files = DragAndDropHelper::supportedTracksFromUrls(
                mimeData.urls(), firstOnly, acceptPlaylists);
        return files;
    }


    static QDrag* dragTrack(TrackPointer pTrack, QWidget* pDragSource,
                            QString sourceIdentifier) {
        QList<QUrl> locationUrls;
        locationUrls.append(urlFromLocation(pTrack->getLocation()));
        return dragUrls(locationUrls, pDragSource, sourceIdentifier);
    }

    static QDrag* dragTrackLocations(const QList<QString>& locations,
                                     QWidget* pDragSource,
                                     QString sourceIdentifier) {
        QList<QUrl> locationUrls;
        foreach (QString location, locations) {
            locationUrls.append(urlFromLocation(location));
        }
        return dragUrls(locationUrls, pDragSource, sourceIdentifier);
    }

    static QUrl urlFromLocation(const QString& trackLocation) {
        return QUrl::fromLocalFile(trackLocation);
    }

  private:
    static QDrag* dragUrls(const QList<QUrl>& locationUrls,
                           QWidget* pDragSource, QString sourceIdentifier) {
        if (locationUrls.isEmpty()) {
            return NULL;
        }

        QMimeData* mimeData = new QMimeData();
        mimeData->setUrls(locationUrls);
        mimeData->setText(sourceIdentifier);

        QDrag* drag = new QDrag(pDragSource);
        drag->setMimeData(mimeData);
        drag->setPixmap(QPixmap(":images/library/ic_library_drag_and_drop.png"));
        drag->exec(Qt::CopyAction);

        return drag;
    }

    static bool addFileToList(const QString& file, QList<QFileInfo>* files) {
        QFileInfo fileInfo(file);

        // Since the user just dropped these files into Mixxx we have permission
        // to touch the file. Create a security token to keep this permission
        // across reboots.
        Sandbox::createSecurityToken(fileInfo);

        if (!fileInfo.exists()) {
            return false;
        }

        // Filter out invalid URLs (eg. files that aren't supported audio
        // filetypes, etc.)
        if (!SoundSourceProxy::isFilenameSupported(fileInfo.fileName())) {
            return false;
        }

        files->append(fileInfo);
        return true;
    }
};

#endif /* DND_H */
