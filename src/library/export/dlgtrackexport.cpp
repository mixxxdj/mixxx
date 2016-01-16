#include "library/export/dlgtrackexport.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QDesktopServices>
#include <QMessageBox>

#include "util/assert.h"


DlgTrackExport::DlgTrackExport(QWidget *parent,
                               ConfigObject<ConfigValue>* pConfig,
                               QList<QString> filenames)
        : QDialog(parent),
          Ui::DlgTrackExport(),
          m_pConfig(pConfig),
          m_filenames(filenames) {
    setupUi(this);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelButtonClicked()));
    exportProgress->setMinimum(0);
    exportProgress->setMaximum(filenames.size());
    exportProgress->setValue(0);
    statusLabel->setText("");
    setModal(true);
}

bool DlgTrackExport::selectDestinationDirectory() {
    QString lastExportDirectory = m_pConfig->getValueString(
            ConfigKey("[Library]", "LastTrackCopyDirectory"),
            QDesktopServices::storageLocation(QDesktopServices::MusicLocation));

    QString destDir = QFileDialog::getExistingDirectory(
        NULL,
        tr("Export Track Files To"),
        lastExportDirectory);
    if (destDir.isNull() || destDir.isEmpty()) {
        // just return, the caller shouldn't be doing any error handling anyway.
        done(QDialog::Accepted);
        return false;
    }
    m_pConfig->set(ConfigKey("[Library]","LastTrackCopyDirectory"),
                ConfigValue(destDir));

    m_exporter.reset(new TrackExport(destDir, m_filenames));
    connect(m_exporter.data(), SIGNAL(progress(QString, int, int)),
            this, SLOT(slotProgress(QString, int, int)));
    connect(m_exporter.data(),
            SIGNAL(askOverwriteMode(QString,
                          std::promise<TrackExport::OverwriteAnswer>*)),
            this,
            SLOT(slotAskOverwriteMode(QString,
                                      std::promise<TrackExport::OverwriteAnswer>*)));
    connect(m_exporter.data(), SIGNAL(canceled()),
            this, SLOT(cancelButtonClicked()));
    return true;
}

void DlgTrackExport::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    DEBUG_ASSERT_AND_HANDLE(m_exporter) {
        qDebug() << "Programming error: did not initialize m_exporter, about to crash";
        return;
    }
    m_exporter->start();
}

void DlgTrackExport::slotAskOverwriteMode(
        QString filename,
        std::promise<TrackExport::OverwriteAnswer>* promise) {
    QMessageBox question_box(
            QMessageBox::Warning,
            tr("Overwrite Existing File?"),
            tr("%1 already exists, overwrite?").arg(filename),
            QMessageBox::Cancel | QMessageBox::No | QMessageBox::NoToAll |
                    QMessageBox::Yes | QMessageBox::YesToAll);
    question_box.setDefaultButton(QMessageBox::No);
    question_box.setButtonText(QMessageBox::Yes, tr("&Overwrite"));
    question_box.setButtonText(QMessageBox::YesToAll, tr("Over&write All"));
    question_box.setButtonText(QMessageBox::No, tr("&Skip"));
    question_box.setButtonText(QMessageBox::NoToAll, tr("Skip &All"));

    switch (question_box.exec()) {
    case QMessageBox::No:
        promise->set_value(TrackExport::OverwriteAnswer::SKIP);
        return;
    case QMessageBox::NoToAll:
        promise->set_value(TrackExport::OverwriteAnswer::SKIP_ALL);
        return;
    case QMessageBox::Yes:
        promise->set_value(TrackExport::OverwriteAnswer::OVERWRITE);
        return;
    case QMessageBox::YesToAll:
        promise->set_value(TrackExport::OverwriteAnswer::OVERWRITE_ALL);
        return;
    case QMessageBox::Cancel:
    default:
        promise->set_value(TrackExport::OverwriteAnswer::CANCEL);
    }
}

void DlgTrackExport::slotProgress(QString filename, int progress, int count) {
    if (progress == count) {
        statusLabel->setText(tr("Export finished"));
        finish();
    } else {
        statusLabel->setText(tr("Exporting %1").arg(filename));
    }
    exportProgress->setMinimum(0);
    exportProgress->setMaximum(count);
    exportProgress->setValue(progress);
}

void DlgTrackExport::finish() {
    m_exporter->stop();
    m_exporter->wait();
    hide();
    if (m_exporter->errorMessage().length()) {
        QMessageBox::warning(
                NULL,
                tr("Export Error"),
                m_exporter->errorMessage(),
                QMessageBox::Ok, QMessageBox::Ok);
    } else {
        QMessageBox::information(
                NULL,
                tr("Complete"),
                tr("Track file export complete"),
                QMessageBox::Ok, QMessageBox::Ok);
    }
    accept();
}

void DlgTrackExport::cancelButtonClicked() {
    finish();
}
