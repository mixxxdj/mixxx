#include "library/samples/dlgsamples.h"

#include <QBoxLayout>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/library.h"
#include "library/trackcollectionmanager.h"
#include "moc_dlgsamples.cpp"
#include "util/assert.h"
#include "widget/wlibrary.h"
#include "widget/wtracktableview.h"

namespace {

// Resolve the samples directory to a writable location.
// On platforms where samples are bundled as read-only resources (Android assets:/),
// this copies them to a writable location so the audio engine can load them.
QString resolveSamplesPath(UserSettingsPointer pConfig) {
    const QString kSamplesSubdir = QStringLiteral("samples/");

    // Candidates for source: resource path first, then fallback
    QStringList sourceCandidates = {
            pConfig->getResourcePath() + kSamplesSubdir,
            QStringLiteral("res/samples/"),
    };

    QString sourceDir;
    for (const auto& candidate : sourceCandidates) {
        QFileInfo fi(candidate);
        if (fi.exists() && fi.isDir()) {
            sourceDir = fi.absoluteFilePath();
            break;
        }
    }

    if (sourceDir.isEmpty()) {
        return QString();
    }

    // Determine writable destination
    QString destDir = QStandardPaths::writableLocation(
                              QStandardPaths::AppLocalDataLocation) +
            QStringLiteral("/") + kSamplesSubdir;
    QDir dest(destDir);
    if (!dest.exists()) {
        dest.mkpath(QStringLiteral("."));
    }

    // Check if already extracted
    QFileInfo destMarker(destDir + QStringLiteral(".extracted"));
    if (destMarker.exists()) {
        return destDir;
    }

    // Copy sample files from source to writable destination
    QDir source(sourceDir);
    QStringList filters = {QStringLiteral("*.mp3"),
            QStringLiteral("*.wav"),
            QStringLiteral("*.aiff"),
            QStringLiteral("*.m4a"),
            QStringLiteral("*.ogg")};
    QFileInfoList files = source.entryInfoList(filters, QDir::Files);

    for (const auto& fi : files) {
        QString destPath = destDir + fi.fileName();
        if (!QFile::exists(destPath)) {
            QFile::copy(fi.absoluteFilePath(), destPath);
        }
    }

    // Create marker to avoid re-extracting
    QFile markerFile(destDir + QStringLiteral(".extracted"));
    markerFile.open(QIODevice::WriteOnly);
    markerFile.close();

    return destDir;
}

} // anonymous namespace

DlgSamples::DlgSamples(
        WLibrary* parent,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        KeyboardEventFilter* pKeyboard)
        : QWidget(parent),
          m_pConfig(pConfig),
          m_pTrackTableView(
                  new WTrackTableView(
                          this,
                          pConfig,
                          pLibrary,
                          parent->getTrackTableBackgroundColorOpacity())),
          m_browseModel(this, pLibrary->trackCollectionManager(), nullptr),
          m_proxyModel(&m_browseModel, true) {
    // Set up layout with just the track table
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_pTrackTableView);

    m_pTrackTableView->installEventFilter(pKeyboard);

    connect(m_pTrackTableView,
            &WTrackTableView::loadTrack,
            this,
            &DlgSamples::loadTrack);
    connect(m_pTrackTableView,
            &WTrackTableView::loadTrackToPlayer,
            this,
            &DlgSamples::loadTrackToPlayer);
    connect(pLibrary,
            &Library::setTrackTableFont,
            m_pTrackTableView,
            &WTrackTableView::setTrackTableFont);
    connect(pLibrary,
            &Library::setTrackTableRowHeight,
            m_pTrackTableView,
            &WTrackTableView::setTrackTableRowHeight);
    connect(pLibrary,
            &Library::setSelectedClick,
            m_pTrackTableView,
            &WTrackTableView::setSelectedClick);
    connect(&m_browseModel,
            &BrowseTableModel::restoreModelState,
            m_pTrackTableView,
            &WTrackTableView::restoreCurrentViewState);

    m_proxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel.setSortCaseSensitivity(Qt::CaseInsensitive);

    refreshBrowseModel();
    m_pTrackTableView->loadTrackModel(&m_proxyModel);
}

DlgSamples::~DlgSamples() {
}

bool DlgSamples::hasFocus() const {
    return m_pTrackTableView->hasFocus();
}

void DlgSamples::setFocus() {
    m_pTrackTableView->setFocus();
}

void DlgSamples::refreshBrowseModel() {
    saveCurrentViewState();
    QString path = resolveSamplesPath(m_pConfig);
    if (!path.isEmpty()) {
        m_browseModel.setPath(mixxx::FileAccess(mixxx::FileInfo(path)));
    }
}

void DlgSamples::onSearch(const QString& text) {
    m_proxyModel.search(text);
}

void DlgSamples::slotRestoreSearch() {
    emit restoreSearch(currentSearch());
}

void DlgSamples::saveCurrentViewState() {
    m_pTrackTableView->saveCurrentViewState();
}

bool DlgSamples::restoreCurrentViewState() {
    return m_pTrackTableView->restoreCurrentViewState();
}