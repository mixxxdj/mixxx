#include "library/export/trackexportwizard.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QDesktopServices>
#include <QMessageBox>

#include "util/assert.h"

void TrackExportWizard::exportTracks() {
    if (!selectDestinationDirectory()) {
        return;
    }
    // Ignore return value, upstream callers don't need it.
    m_dialog->exec();
}

bool TrackExportWizard::selectDestinationDirectory() {
    QString lastExportDirectory = m_pConfig->getValue(
            ConfigKey("[Library]", "LastTrackCopyDirectory"),
            QDesktopServices::storageLocation(QDesktopServices::MusicLocation));

    QString destDir = QFileDialog::getExistingDirectory(
            NULL, tr("Export Track Files To"), lastExportDirectory);
    if (destDir.isEmpty()) {
        return false;
    }
    m_pConfig->set(ConfigKey("[Library]", "LastTrackCopyDirectory"),
                   ConfigValue(destDir));

    m_worker.reset(new TrackExportWorker(destDir, m_tracks));
    m_dialog.reset(new TrackExportDlg(m_parent, m_pConfig, m_worker.data()));
    return true;
}

