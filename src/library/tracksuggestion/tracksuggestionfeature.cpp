#include "library/tracksuggestion/tracksuggestionfeature.h"

#include "library/dao/trackschema.h"
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

// This can be replaced with the XML parse.
const QString kTrack = "track";
const QString kName = "name";
const QString kPlaycount = "playcount";
const QString kMbid = "mbid";
const QString kMatch = "match";
const QString kDuration = "duration";
const QString kArtist = "artist";
const QString kUrl = "url";
const QString kImage = "image";

// Dummy location added: If there is no location for a suggestion, seg fault occurs after pressing on the suggestion.
const QString kLocation = "/localhost/Users/A/Music/Suggestion/Suggestion-Library/Tracks/";

} //namespace

TrackSuggestionFeature::TrackSuggestionFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig)
        : BaseExternalLibraryFeature(pLibrary, pConfig, QStringLiteral("tracksuggestion")),
          m_cancelImport(false),
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
    connect(&m_future_watcher,
            &QFutureWatcher<void>::finished,
            this,
            &TrackSuggestionFeature::onSuggestionFileParsed);
}

TrackSuggestionFeature::~TrackSuggestionFeature() {
    qDebug() << "TrackSuggestionFeature Destructor";
    m_database.close();
    m_cancelImport = true;
    m_future.waitForFinished();
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
                    m_pSuggestionPlaylistModel);
    libraryWidget->registerView(kSuggestionTitle, pTrackSuggestionView);

    connect(pTrackSuggestionView,
            &DlgTrackSuggestion::buttonPressed,
            this,
            &TrackSuggestionFeature::slotStartFetchingViaButton);

    connect(pTrackSuggestionView,
            &DlgTrackSuggestion::suggestionFileWrittenSuccessfully,
            this,
            &TrackSuggestionFeature::slotUpdateTrackModelAfterSuccess);
}

void TrackSuggestionFeature::htmlLinkClicked(const QUrl& link) {
    if (QString(link.path()) == "refresh") {
        activate();
    } else {
        qDebug() << "Unknown link clicked" << link;
    }
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
    QString selectedDeckItemTrackLocation = trackPointer->getLocation();

    if (selectedDeckItemTrackLocation.isNull()) {
        qDebug() << "Selected track suggestion item is null.";
        emit switchToView(kSuggestionTitle);
    }

    if (!selectedDeckItemTrackLocation.isNull()) {
        QFileInfo trackLocation(selectedDeckItemTrackLocation);
        QString suggestionFileLocation = trackLocation.absoluteFilePath() + ".xml";

        m_suggestionFile = suggestionFileLocation;

        emit showTrackModel(m_pSuggestionTrackModel);

        qDebug() << "Suggestion file location is " << suggestionFileLocation;

        if (QFile::exists(m_suggestionFile)) {
            qDebug() << "File::exists.";
        } else {
            emit switchToView(kSuggestionTitle);
            qDebug() << "Suggestion file doesn't exists";
            return;
        }

        mixxx::FileInfo fileInfo(m_suggestionFile);
        if (fileInfo.checkFileExists()) {
            //Delete all table entries of Suggestion feature

            Sandbox::askForAccess(&fileInfo);
        }
    }
// Let a worker thread do the XML parsing
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_future = QtConcurrent::run(&TrackSuggestionFeature::parseSuggestionFile, this);
#else
    m_future = QtConcurrent::run(this, &TrackSuggestionFeature::parseSuggestionFile);
#endif
    m_future_watcher.setFuture(m_future);
    emit enableCoverArtDisplay(false);
}

void TrackSuggestionFeature::parseSuggestionFile() {
    int suggestionId = 0;
    QString title;
    QString artist;
    int playcount = 0;
    float match = 0;
    float duration = 0;

    ScopedTransaction transaction(m_database);
    clearTable("suggestion_library");
    transaction.commit();

    QSqlQuery query(m_database);
    query.prepare(
            "INSERT INTO suggestion_library(id, artist, title, location, "
            "duration, match, playcount) "
            "VALUES (:id, :artist, :title, :location, :duration, :match, "
            ":playcount)");

    QFile suggestion_file(m_suggestionFile);
    if (!suggestion_file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open Suggestion XML file";
    }

    QXmlStreamReader xml(&suggestion_file);

    while (!xml.atEnd() && !m_cancelImport) {
        xml.readNext();

        if (xml.name() == QLatin1String("track")) {
            while (true) {
                xml.readNext();
                if (xml.name() == QLatin1String("track") && xml.isEndElement()) {
                    break;
                }
                if (xml.isStartElement() && xml.name() == QLatin1String("name")) {
                    title = xml.readElementText();
                    //qDebug() << "This xml element is 'name' of the suggestion:" << title;
                }
                if (xml.isStartElement() && xml.name() == QLatin1String("playcount")) {
                    playcount = xml.readElementText().toInt();
                    //qDebug() << "This xml element is 'playcount' of the suggestion:" << playcount;
                }
                if (xml.isStartElement() && xml.name() == QLatin1String("match")) {
                    match = xml.readElementText().toFloat();
                    //qDebug() << "This xml element is 'match' of the suggestion:" << match;
                }
                if (xml.isStartElement() && xml.name() == QLatin1String("duration")) {
                    duration = xml.readElementText().toFloat();
                    //qDebug() << "This xml element is 'duration' of the suggestion:" << duration;
                }
                if (xml.isStartElement() && xml.name() == QLatin1String("artist")) {
                    xml.readNext();
                    while (xml.name() != QLatin1String("artist")) {
                        if (xml.name() == QLatin1String("name")) {
                            artist = xml.readElementText();
                            //qDebug() << "This xml element is 'artist' of the suggestion:" << artist;
                        }
                        xml.readNext();
                    }
                }
            }
            suggestionId += 1;
            query.bindValue(":id", suggestionId);
            query.bindValue(":artist", artist);
            query.bindValue(":title", title);
            query.bindValue(":duration", duration);
            query.bindValue(":match", match);
            query.bindValue(":playcount", playcount);
            query.bindValue(":location", kLocation);

            bool success = query.exec();

            if (!success) {
                LOG_FAILED_QUERY(query);
                return;
            }
        }
    }
    //Last.fm returns a response without any suggestions with just an track name.
    //In this circumstances, this is block is added for initial PR.
    //Later on this can be handled on task.
    if (suggestionId < 1) {
        query.bindValue(":id", suggestionId);
        query.bindValue(":artist", "No Suggestion");
        query.bindValue(":title", "Found for this track.");
        query.bindValue(":location", kLocation);

        bool success = query.exec();

        if (!success) {
            LOG_FAILED_QUERY(query);
            return;
        }
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
    m_pTrack = pNewTrack;
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

void TrackSuggestionFeature::clearTable(const QString& table_name) {
    QSqlQuery query(m_database);
    query.prepare("delete from " + table_name);
    bool success = query.exec();

    if (!success) {
        qDebug() << "Could not delete remove old entries from table "
                 << table_name << " : " << query.lastError();
    } else {
        qDebug() << "Suggestion table entries of '"
                 << table_name << "' have been cleared.";
    }
}

void TrackSuggestionFeature::onSuggestionFileParsed() {
    m_trackSource->buildIndex();
    emit showTrackModel(m_pSuggestionTrackModel);
    qDebug() << "Suggestion library loaded successfully.";
}

void TrackSuggestionFeature::slotStartFetchingViaButton() {
    m_pSuggestionFetcher->startFetch(m_pTrack);
    qDebug() << "Fetching started";
}

void TrackSuggestionFeature::emitTrackPropertiesToDialog(TrackPointer pTrack) {
    m_pSuggestionFetcher->transmitButtonLabel(pTrack);
}

void TrackSuggestionFeature::slotUpdateTrackModelAfterSuccess(const QString& filePath) {
    m_suggestionFile = filePath;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_future = QtConcurrent::run(&TrackSuggestionFeature::parseSuggestionFile, this);
#else
    m_future = QtConcurrent::run(this, &TrackSuggestionFeature::parseSuggestionFile);
#endif
    m_future_watcher.setFuture(m_future);
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
        QString trackLocation = trackPointer->getLocation();
        treeItemSelectedTrack->setLabel(composeTreeItemLabel(artist, title));
        treeItemSelectedTrack->setData(m_selectedTrackId.toVariant());
    }
}
