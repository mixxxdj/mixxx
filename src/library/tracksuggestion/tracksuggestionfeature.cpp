#include "library/tracksuggestion/tracksuggestionfeature.h"

#include "library/library.h"
#include "library/queryutil.h"
#include "library/trackcollectionmanager.h"
#include "library/treeitem.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "track/track.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarytextbrowser.h"

namespace {

const QString composeTreeItemLabel(const QString& track,
        const QString& artist) {
    return track + QStringLiteral(" | ") + artist;
}

const QString kSuggestionTitle("SuggestionView");

const QString kNoSuggestionAvailable("NoSuggestionAvailable");


// Dummy location added: If there is no location for a suggestion, seg fault occurs after pressing on the suggestion.
const QString kLocation =
        "/localhost/Users/A/Music/Suggestion/Suggestion-Library/Tracks/";

} //namespace

TrackSuggestionFeature::TrackSuggestionFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig)
        : BaseExternalLibraryFeature(pLibrary, pConfig, QStringLiteral("tracksuggestion")),
          m_pSidebarModel(make_parented<TreeItemModel>(this)),
          m_pSuggestionFetcher(make_parented<SuggestionFetcher>(this)) {
    m_title = tr("Track Suggestion");

    connect(&PlayerInfo::instance(),
            &PlayerInfo::trackChanged,
            this,
            &TrackSuggestionFeature::slotTrackChanged);

    connect(pLibrary,
            &Library::trackSelected,
            this,
            [this](const TrackPointer& pTrack) {
                const auto trackId = pTrack ? pTrack->getId() : TrackId{};
                slotTrackSelected(trackId);
            });

    QString tableName = "suggestion_library";
    QString idColumn = "id";
    QStringList columns;
    columns << "id"
            << "artist"
            << "title"
            << "playcount"
            << "match"
            << "duration"
            << "location";

    m_trackSource = QSharedPointer<BaseTrackCache>(new BaseTrackCache(
            m_pLibrary->trackCollectionManager()->internalCollection(),
            tableName,
            idColumn,
            columns,
            false));

    m_pSuggestionTrackModel = new BaseExternalTrackModel(this,
            m_pLibrary->trackCollectionManager(),
            "mixxx.db.model.suggestion",
            "suggestion_library",
            m_trackSource);

    m_pSuggestionPlaylistModel = new BaseExternalPlaylistModel(this,
            m_pLibrary->trackCollectionManager(),
            "mixxx.db.model.suggestion_playlist",
            "suggestion_playlists",
            "suggestion_playlist_tracks",
            m_trackSource);

    //This is added for the correct behaviour of DlgTrackSuggestion for now
    m_DummyModel = new BaseExternalPlaylistModel(this,
            m_pLibrary->trackCollectionManager(),
            "mixxx.db",
            "suggestion",
            "suggestion",
            m_trackSource);

    m_database = QSqlDatabase::cloneDatabase(m_pLibrary->trackCollectionManager()
                                                     ->internalCollection()
                                                     ->database(),
            "SUGGESTION_SCANNER");

    if (!m_database.open()) {
        qDebug() << "Failed to open database for Suggestion scanner." << m_database.lastError();
    }

    std::unique_ptr<TreeItem> pRootItem = TreeItem::newRoot(this);
    treeItemSelectedTrack = pRootItem->appendChild(tr("Selected Track"));
    treeItemDeckOne = pRootItem->appendChild(tr("Deck 1"));
    treeItemDeckTwo = pRootItem->appendChild(tr("Deck 2"));
    treeItemDeckThree = pRootItem->appendChild(tr("Deck 3"));
    treeItemDeckFour = pRootItem->appendChild(tr("Deck 4"));
    m_pSidebarModel->setRootItem(std::move(pRootItem));
}

TrackSuggestionFeature::~TrackSuggestionFeature() {
    qDebug() << "TrackSuggestionFeature Destructor";
    m_database.close();
    delete m_pSuggestionTrackModel;
}

void TrackSuggestionFeature::bindLibraryWidget(WLibrary* libraryWidget,
        KeyboardEventFilter* keyboard) {
    Q_UNUSED(keyboard);
    WLibraryTextBrowser* edit = new WLibraryTextBrowser(libraryWidget);
    edit->setHtml(formatRootViewHtml());
    edit->setOpenLinks(false);
    connect(edit,
            &WLibraryTextBrowser::anchorClicked,
            this,
            &TrackSuggestionFeature::htmlLinkClicked);
    libraryWidget->registerView("TRACKSUGGESTION", edit);

    DlgTrackSuggestion* pTrackSuggestionView =
            new DlgTrackSuggestion(libraryWidget,
                    m_pConfig,
                    m_pLibrary,
                    keyboard,
                    m_pSuggestionFetcher,
                    m_DummyModel);
    libraryWidget->registerView(kSuggestionTitle, pTrackSuggestionView);

    connect(pTrackSuggestionView,
            &DlgTrackSuggestion::buttonPressed,
            this,
            &TrackSuggestionFeature::slotStartFetchingViaButton);

    connect(pTrackSuggestionView,
            &DlgTrackSuggestion::suggestionResults,
            this,
            &TrackSuggestionFeature::slotUpdateTrackModelAfterSuccess);

    WLibraryTextBrowser* suggestionNotFound = new WLibraryTextBrowser(libraryWidget);
    suggestionNotFound->setHtml(formatNoSuggestionAvailableHtml());
    suggestionNotFound->setOpenLinks(false);
    connect(suggestionNotFound,
            &WLibraryTextBrowser::anchorClicked,
            this,
            &TrackSuggestionFeature::htmlLinkClicked);
    libraryWidget->registerView(kNoSuggestionAvailable, suggestionNotFound);
}

void TrackSuggestionFeature::htmlLinkClicked(const QUrl& link) {
    if (QString(link.path()) == "refresh") {
        activate();
    } else {
        qDebug() << "Unknown link clicked" << link;
    }
}

BaseSqlTableModel* TrackSuggestionFeature::getPlaylistModelForPlaylist(const QString& playlist) {
    BaseExternalPlaylistModel* pModel = new BaseExternalPlaylistModel(this,
            m_pLibrary->trackCollectionManager(),
            "mixxx.db.model.suggestion_playlist",
            "suggestion_playlists",
            "suggestion_playlist_tracks",
            m_trackSource);
    pModel->setPlaylist(playlist);
    return pModel;
}

QVariant TrackSuggestionFeature::title() {
    return m_title;
}

bool TrackSuggestionFeature::isSupported() {
    return true;
}

TreeItemModel* TrackSuggestionFeature::sidebarModel() const {
    return m_pSidebarModel;
}

QString TrackSuggestionFeature::formatRootViewHtml() const {
    QString title = tr("Track Suggestion Feature");
    QString summary =
            tr("Track suggestion is a helper feature to get suggestion of "
               "tracks on the desk playing. It uses Last.fm get similar tracks "
               "API and returns similar tracks.");
    QStringList items;

    items << tr("Get similar tracks")
          << tr("Get similar artists");

    QString html;
    // TODO: This can show a pop up about how to use this feature very detailed.
    QString refreshLink = tr("For additional info:");
    html.append(QString("<h2>%1</h2>").arg(title));
    html.append(QString("<p>%1</p>").arg(summary));
    html.append(QString("<ul>"));
    for (const auto& item : qAsConst(items)) {
        html.append(QString("<li>%1</li>").arg(item));
    }
    html.append(QString("</ul>"));
    html.append(QString("<a style=\"color:#0496FF;\" href=\"refresh\">%1</a>")
                        .arg(refreshLink));

    // TODO: add "powered by AudioScrobbler" image mentioned on the last.fm terms of service.
    // See 2.7 at:
    // https://www.last.fm/api/tos

    // QUESTION: Inform user about the laws (also mentioned on TOS) before using this service?
    return html;
}

QString TrackSuggestionFeature::formatNoSuggestionAvailableHtml() const {
    QString title = tr("Track Suggestions Not Found");
    QString status = tr("There is no suggested tracks found on last.fm.");
    QString updateMessage = tr("We suggest to update the metadata of the track from Musicbrainz.");
    QString propertyMessage = tr("Update the title and artist if they are misspelled.");

    QStringList items;

    items << tr("Import metadata from MusicBrainz")
          << tr("Update the track properties");

    QString html;
    html.append(QString("<h2>%1</h2>").arg(title));
    html.append(QString("<p>%1</p>").arg(status));
    html.append(QString("<p>%1</p>").arg(updateMessage));
    html.append(QString("<p>%1</p>").arg(propertyMessage));

    return html;
}

void TrackSuggestionFeature::activate() {
    m_title = tr("Track Suggestion");
    emit disableSearch();
    emit enableCoverArtDisplay(false);
    emit switchToView("TRACKSUGGESTION");
}

void TrackSuggestionFeature::activateChild(const QModelIndex& index) {
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (!(item && item->getData().isValid())) {
        return;
    }
    qDebug() << "TrackSuggestionFeature item" << item->getLabel()
             << "selected. The track ID is: " << item->getData();
    TrackId trackId = TrackId(item->getData());
    if (!trackId.isValid()) {
        qDebug() << "Track ID is not valid";
        return;
    }
    TrackPointer trackPointer = m_pLibrary->trackCollectionManager()->getTrackById(trackId);
    emitTrackPropertiesToDialog(trackPointer);
    m_pTrack = trackPointer;
    if (!lookTrackHasSuggestions(trackId)) {
        qDebug() << "Selected track has no suggestions fetched before.";
        emit switchToView(kSuggestionTitle);
    } else {
        m_pSuggestionPlaylistModel->setPlaylist(trackId.toString());
        emit showTrackModel(m_pSuggestionPlaylistModel);
        emit enableCoverArtDisplay(false);
    }
}

void TrackSuggestionFeature::slotTrackChanged(const QString& group,
        TrackPointer pNewTrack,
        TrackPointer pOldTrack) {
    if (pNewTrack) {
        playerInfoTrackLoaded(group, pNewTrack);
    }
}

void TrackSuggestionFeature::playerInfoTrackLoaded(const QString& group, TrackPointer pNewTrack) {
    QString artist = pNewTrack->getArtist();
    QString title = pNewTrack->getTitle();
    TrackId trackId = pNewTrack->getId();
    if (!trackId.isValid()) {
        qDebug() << "Track ID is not valid!";
        return;
    }
    if (group == "[Channel1]") {
        treeItemDeckOne->setLabel(composeTreeItemLabel(artist, title));
        treeItemDeckOne->setData(trackId.toVariant());
    } else if (group == "[Channel2]") {
        treeItemDeckTwo->setLabel(composeTreeItemLabel(artist, title));
        treeItemDeckTwo->setData(trackId.toVariant());
    } else if (group == "[Channel3]") {
        treeItemDeckThree->setLabel(composeTreeItemLabel(artist, title));
        treeItemDeckThree->setData(trackId.toVariant());
    } else {
        treeItemDeckFour->setLabel(composeTreeItemLabel(artist, title));
        treeItemDeckFour->setData(trackId.toVariant());
    }
}

void TrackSuggestionFeature::slotStartFetchingViaButton() {
    m_pSuggestionFetcher->startFetch(m_pTrack);
    qDebug() << "Fetching started";
}

void TrackSuggestionFeature::emitTrackPropertiesToDialog(TrackPointer pTrack) {
    m_pSuggestionFetcher->transmitButtonLabel(pTrack);
}

void TrackSuggestionFeature::slotUpdateTrackModelAfterSuccess(
        const QList<QMap<QString, QString>>& suggestions) {
    m_suggestions = suggestions;
    QSqlQuery query(m_database);
    query.prepare(
            "INSERT INTO suggestion_library(artist, title, location, match, duration, playcount) "
            "VALUES (:artist, :title, :location, :match, :duration, :playcount)");

    QSqlQuery queryInsertToPlaylist(m_database);
    queryInsertToPlaylist.prepare(
            "INSERT INTO suggestion_playlists (id, name) "
            "VALUES (:id, :name)");

    // position is added for the base class.
    QSqlQuery queryInsertToPlaylistTracks(m_database);
    queryInsertToPlaylistTracks.prepare(
            "INSERT INTO suggestion_playlist_tracks (playlist_id, track_id, "
            "position) "                                    // track_id is actually a suggestion_id
            "VALUES (:playlist_id, :track_id, :position)"); // track_id is actually a suggestion_id

    QString artist;
    QString title;
    QString playcount;
    QString match;
    QString duration;
    QString trackId = m_pTrack->getId().toString();
    int position = 0;

    QString playlistName = composeTreeItemLabel(m_pTrack->getTitle(), m_pTrack->getArtist());

    for (const auto& resultTrack : m_suggestions) {
        artist = resultTrack.value("artist");
        title = resultTrack.value("title");
        playcount = resultTrack.value("playcount");
        match = resultTrack.value("match");
        duration = resultTrack.value("duration");

        query.bindValue(":artist", artist);
        query.bindValue(":title", title);
        query.bindValue(":match", match);
        query.bindValue(":duration", duration);
        query.bindValue(":playcount", playcount);
        query.bindValue(":location", kLocation);

        bool success = query.exec();
        if (!success) {
            emit switchToView(kNoSuggestionAvailable);
            LOG_FAILED_QUERY(query);
            return;
        }

        position++;
        queryInsertToPlaylistTracks.bindValue(":playlist_id", trackId);
        queryInsertToPlaylistTracks.bindValue(":track_id",
                query.lastInsertId()
                        .toInt()); // track_id is actually a suggestion_id
        queryInsertToPlaylistTracks.bindValue(":position", position);

        success = queryInsertToPlaylistTracks.exec();
        if (!success) {
            LOG_FAILED_QUERY(queryInsertToPlaylistTracks);
            emit switchToView(kNoSuggestionAvailable);
            return;
        }
    }

    // Playlist name is set to TrackID to lookup.
    queryInsertToPlaylist.bindValue(":id", trackId);
    queryInsertToPlaylist.bindValue(":name", QString(trackId));

    bool success = queryInsertToPlaylist.exec();
    if (!success) {
        LOG_FAILED_QUERY(queryInsertToPlaylist);
        emit switchToView(kNoSuggestionAvailable);
        return;
    }

    m_trackSource->buildIndex();
    m_pSuggestionPlaylistModel->setPlaylist(trackId);
    emit showTrackModel(m_pSuggestionPlaylistModel);
};

void TrackSuggestionFeature::slotTrackSelected(TrackId trackId) {
    m_selectedTrackId = trackId;
    if (m_selectedTrackId.isValid()) {
        TrackPointer trackPointer =
                m_pLibrary->trackCollectionManager()->getTrackById(
                        m_selectedTrackId);
        emitTrackPropertiesToDialog(trackPointer);
        QString artist = trackPointer->getArtist();
        QString title = trackPointer->getTitle();
        treeItemSelectedTrack->setLabel(composeTreeItemLabel(artist, title));
        treeItemSelectedTrack->setData(m_selectedTrackId.toVariant());
    }
}

bool TrackSuggestionFeature::lookTrackHasSuggestions(TrackId trackid) {
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
            "SELECT COUNT(*) FROM suggestion_playlist_tracks WHERE playlist_id=%1")
                          .arg(trackid.toString()));
    query.exec();
    query.first();
    qDebug() << query.value(0).toInt();
    if (query.value(0).toInt() < 1) {
        qDebug() << "Suggestions is not found in database.";
        return false;
    } else {
        qDebug() << "Suggestions found in the database.";
        return true;
    }
}
