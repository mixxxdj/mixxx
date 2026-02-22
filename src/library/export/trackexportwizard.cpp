#include "library/export/trackexportwizard.h"

#include <QFileDialog>
#include <QStandardPaths>

#include "moc_trackexportwizard.cpp"

void TrackExportWizard::exportTracks() {
    if (!selectDestinationDirectory()) {
        return;
    }
    // Ignore return value, upstream callers don't need it.
    m_dialog->exec();
}

bool TrackExportWizard::selectDestinationDirectory() {
    if (m_tracks.isEmpty()) {
        qInfo() << "TrackExportWizard: No tracks to export, cancel.";
        return false;
    }

    QString lastExportDirectory = m_pConfig->getValue(
            ConfigKey("[Library]", "LastTrackCopyDirectory"),
            QStandardPaths::writableLocation(QStandardPaths::MusicLocation));

    QString destDir = QFileDialog::getExistingDirectory(
            nullptr, tr("Export Track Files To"), lastExportDirectory);
    if (destDir.isEmpty()) {
        return false;
    }
    m_pConfig->set(ConfigKey("[Library]", "LastTrackCopyDirectory"),
                   ConfigValue(destDir));

    m_worker.reset(new TrackExportWorker(destDir, m_tracks));
    m_dialog.reset(new TrackExportDlg(m_parent, m_pConfig, m_worker.data()));
    return true;
}
