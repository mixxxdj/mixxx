#include "library/scanner/libraryscannerdlg.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtDebug>

#include "moc_libraryscannerdlg.cpp"

LibraryScannerDlg::LibraryScannerDlg(QWidget* parent, Qt::WindowFlags f)
        : QWidget(parent, f),
          m_bCancelled(false) {
    setWindowIcon(QIcon(":/images/mixxx_icon.svg"));

    QVBoxLayout* pLayout = new QVBoxLayout(this);

    setWindowTitle(tr("Library Scanner"));
    QLabel* pLabel = new QLabel(tr("It's taking Mixxx a minute to scan your music library, please wait..."),this);
    pLayout->addWidget(pLabel);

    QPushButton* pCancel = new QPushButton(tr("Cancel"), this);
    connect(pCancel,
            &QPushButton::clicked,
            this,
            &LibraryScannerDlg::slotCancel);
    pLayout->addWidget(pCancel);

    QLabel* pCurrent = new QLabel(this);
    pCurrent->setAlignment(Qt::AlignTop);
    pCurrent->setMaximumWidth(600);
    pCurrent->setFixedHeight(this->fontMetrics().height());
    pCurrent->setWordWrap(true);
    connect(this, &LibraryScannerDlg::progress, pCurrent, &QLabel::setText);
    pLayout->addWidget(pCurrent);
    setLayout(pLayout);
}

LibraryScannerDlg::~LibraryScannerDlg() {
}

void LibraryScannerDlg::slotUpdate(const QString& path) {
    //qDebug() << "LibraryScannerDlg slotUpdate" << m_timer.elapsed().formatMillisWithUnit() << path;
    if (!m_bCancelled && m_timer.elapsed() > mixxx::Duration::fromSeconds(2)) {
       setVisible(true);
    }

    if (isVisible()) {
        QString status = tr("Scanning: ") + path;
        emit progress(status);
    }
}

void LibraryScannerDlg::slotUpdateCover(const QString& path) {
    //qDebug() << "LibraryScannerDlg slotUpdate" << m_timer.elapsed() << path;
    if (!m_bCancelled && m_timer.elapsed() > mixxx::Duration::fromSeconds(2)) {
       setVisible(true);
    }

    if (isVisible()) {
        QString status = QString("%1: %2").arg(tr("Scanning cover art (safe to cancel)"), path);
        emit progress(status);
    }
}

void LibraryScannerDlg::slotCancel() {
    qDebug() << "Cancelling library scan...";
    m_bCancelled = true;
    emit scanCancelled();
    hide();
}

void LibraryScannerDlg::slotScanStarted() {
    m_bCancelled = false;
    m_timer.start();
}

void LibraryScannerDlg::slotScanFinished() {
    // Raise this flag to prevent any latent slotUpdates() from showing the
    // dialog again.
    m_bCancelled = true;

    hide();
}
