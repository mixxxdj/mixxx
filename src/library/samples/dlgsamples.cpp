#include "library/samples/dlgsamples.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QVBoxLayout>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/library.h"
#include "library/trackcollectionmanager.h"
#include "moc_dlgsamples.cpp"
#include "track/track.h"
#include "util/assert.h"
#include "widget/wlibrary.h"

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
    const QFileInfoList files = source.entryInfoList(filters, QDir::Files);

    for (const auto& fi : std::as_const(files)) {
        QString destPath = destDir + fi.fileName();
        if (!QFile::exists(destPath)) {
            if (!QFile::copy(fi.absoluteFilePath(), destPath)) {
                qWarning() << "Failed to copy sample:"
                           << fi.absoluteFilePath();
            }
        }
    }

    // Create marker to avoid re-extracting
    QFile markerFile(destDir + QStringLiteral(".extracted"));
    if (!markerFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to create .extracted marker file:"
                   << destDir + QStringLiteral(".extracted");
    }
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
          m_pLibrary(pLibrary),
          m_pSampleList(new QListWidget(this)) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_pSampleList);

    m_pSampleList->installEventFilter(pKeyboard);

    connect(m_pSampleList,
            &QListWidget::itemActivated,
            this,
            &DlgSamples::slotSampleActivated);

    refreshBrowseModel();
}

DlgSamples::~DlgSamples() {
}

void DlgSamples::refreshBrowseModel() {
    m_pSampleList->clear();
    m_sampleFiles.clear();
    m_samplesPath = resolveSamplesPath(m_pConfig);

    if (m_samplesPath.isEmpty()) {
        qWarning() << "Samples path is empty, no samples to display";
        return;
    }

    QDir dir(m_samplesPath);
    QStringList filters = {QStringLiteral("*.mp3"),
            QStringLiteral("*.wav"),
            QStringLiteral("*.aiff"),
            QStringLiteral("*.m4a"),
            QStringLiteral("*.ogg")};
    m_sampleFiles = dir.entryList(filters, QDir::Files, QDir::Name);

    for (const auto& filename : m_sampleFiles) {
        QListWidgetItem* pItem = new QListWidgetItem(filename);
        pItem->setToolTip(dir.absoluteFilePath(filename));
        m_pSampleList->addItem(pItem);
    }
}

void DlgSamples::slotSampleActivated(QListWidgetItem* pItem) {
    if (!pItem) {
        return;
    }

    QString filePath = m_samplesPath + pItem->text();
    QFileInfo fi(filePath);
    if (!fi.exists()) {
        qWarning() << "Sample file not found:" << filePath;
        return;
    }

    // Create a Track object for the sample file and load it to Sampler1
    TrackPointer pTrack = Track::newTemporary(filePath);
    if (!pTrack) {
        qWarning() << "Failed to create Track for sample:" << filePath;
        return;
    }

    emit loadTrackToPlayer(pTrack,
            QStringLiteral("[Sampler1]"),
#ifdef __STEM__
            mixxx::StemChannelSelection(),
#endif
            false);
}

void DlgSamples::onSearch(const QString& text) {
    m_currentSearch = text;
    for (int i = 0; i < m_pSampleList->count(); ++i) {
        QListWidgetItem* pItem = m_pSampleList->item(i);
        pItem->setHidden(!text.isEmpty() &&
                !pItem->text().contains(text, Qt::CaseInsensitive));
    }
}

void DlgSamples::slotRestoreSearch() {
    emit restoreSearch(currentSearch());
}

void DlgSamples::saveCurrentViewState() {
    // No persistent state to save for the list view
}

bool DlgSamples::restoreCurrentViewState() {
    return true;
}

bool DlgSamples::hasFocus() const {
    return m_pSampleList->hasFocus();
}

void DlgSamples::setFocus() {
    m_pSampleList->setFocus();
}
