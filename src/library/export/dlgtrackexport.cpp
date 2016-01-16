#include "library/export/dlgtrackexport.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QDesktopServices>
#include <QMessageBox>


// todo:  how do callers figure out success / failure?
// how do callers block?  I assume there's a method to make a qdialog modal
// how does this dialog block on the thread but not halt the ui?  look at
// controller learning.  something about eventfilter or something
// oh duh probably launch a timer.  have to poll for error condition etc.
// blocking on destruction of other thread is no good.

DlgTrackExport::DlgTrackExport(QWidget *parent,
                               ConfigObject<ConfigValue>* pConfig,
                               QList<QString> filenames)
        : QDialog(parent) {
    QString lastExportDirectory = pConfig->getValueString(
            ConfigKey("[Library]", "LastTrackCopyDirectory"),
            QDesktopServices::storageLocation(QDesktopServices::MusicLocation));

    QString destDir = QFileDialog::getExistingDirectory(
        NULL,
        tr("Export Track Files To"),
        lastExportDirectory);
    if (destDir.isNull() || destDir.isEmpty()) {
        // failed instantiation... this is no good
        return;
    }
    pConfig->set(ConfigKey("[Library]","LastTrackCopyDirectory"),
                ConfigValue(destDir));

    setQuestionShown(false);

    connect(skipButton, SIGNAL(clicked()), this, SLOT(skipButtonClicked()));
    connect(skipAllButton, SIGNAL(clicked()), this, SLOT(skipAllButtonClicked()));
    connect(overwriteButton, SIGNAL(clicked()), this, SLOT(overwriteButtonClicked()));
    connect(overwriteAllButton, SIGNAL(clicked()), this, SLOT(overwriteAllButtonClicked()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelButtonClicked()));

    m_exporter.reset(new TrackExport(destDir));

    // Launch a thread!
    m_exporter->exportTrackList(filenames);
}

void DlgTrackExport::slotAskOverwriteMode(
        QString filename,
        std::promise<TrackExport::OverwriteAnswer>* promise) {
    // TEMP TEMP TEMP
    int ret = QMessageBox::warning(
            NULL, tr("Overwrite Existing File?"),
            tr("%1 already exists, overwrite?").arg(filename),
            QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No,
            QMessageBox::No);

    switch (ret) {
    case QMessageBox::No:
        promise->set_value(TrackExport::OverwriteAnswer::SKIP);
        return;
    case QMessageBox::Yes:
        promise->set_value(TrackExport::OverwriteAnswer::OVERWRITE);
        return;
    case QMessageBox::YesToAll:
        promise->set_value(TrackExport::OverwriteAnswer::OVERWRITE_ALL);
        return;
    default:
        promise->set_value(TrackExport::OverwriteAnswer::CANCEL);
    }
}

void DlgTrackExport::slotProgress(int progress, int count) {
    exportProgress->setMinimum(0);
    exportProgress->setMaximum(count);
    exportProgress->setValue(progress);
}

void DlgTrackExport::skipButtonClicked() {
}

void DlgTrackExport::skipAllButtonClicked() {
}

void DlgTrackExport::overwriteButtonClicked() {
}

void DlgTrackExport::overwriteAllButtonClicked() {
}

void DlgTrackExport::cancelButtonClicked() {
}

void DlgTrackExport::setQuestionShown(bool show) {
    if (show) {
        skipButton->show();
        skipAllButton->show();
        overwriteButton->show();
        overwriteAllButton->show();
    } else {
        skipButton->hide();
        skipAllButton->hide();
        overwriteButton->hide();
        overwriteAllButton->hide();
    }
}
