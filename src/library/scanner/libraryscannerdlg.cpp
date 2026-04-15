#include "library/scanner/libraryscannerdlg.h"

#include <QPushButton>
#include <QVBoxLayout>

#include "defs_urls.h"
#include "moc_libraryscannerdlg.cpp"

LibraryScannerDlg::LibraryScannerDlg(QWidget* pParent)
        : QDialog(pParent),
          m_pLabelCurrent(make_parented<QLabel>(this)),
          m_pProgressBar(make_parented<QProgressBar>(this)),
          m_bCancelled(false),
          m_tasksDone(0),
          m_tasksTotal(0),
          m_showNoTasksQueuedWarning(true) {
    setWindowIcon(QIcon(MIXXX_ICON_PATH));
    setWindowTitle(tr("Library Scanner"));

    // This is the widest item initially so it sets the dialog's minimum width.
    auto pLabel = make_parented<QLabel>(
            tr("It's taking Mixxx a minute to scan your music library, please wait..."),
            this);

    // Add some margin between label and progress bar
    auto pSpacer = make_parented<QWidget>(this);
    pSpacer->setFixedSize(10, 10);

    m_pLabelCurrent->setAlignment(Qt::AlignTop);
    m_pLabelCurrent->setWordWrap(true);
    // Allow the label to expand horizontally if the dialog is expanded manually
    // and vertically if required by the text. `Ignored` means the label will not
    // force-expand the dialog.
    m_pLabelCurrent->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::MinimumExpanding);

    // Try not let the dialog expand beyond the screen's bottom edge, this would
    // push the Cancel button off screen.
    // The dialog is initially shown centered with a height of ~180 px. If we
    // assume a minimum screen height of 768 px, 400 px seems to be safe.
    // Tests on 1920x1080, 96 dpi with system default font (11 pt) have shown
    // that the label can show moore than 7 lines before it is clamped. If users
    // really need to see the full path then they can expand the dialog horizontally.
    setMaximumHeight(400);

    auto pCancel = make_parented<QPushButton>(tr("Cancel"), this);
    connect(pCancel.get(),
            &QPushButton::clicked,
            this,
            &LibraryScannerDlg::slotCancel);
    // Also cancel when closing by click on X window button
    connect(this,
            &QDialog::rejected,
            this,
            &LibraryScannerDlg::slotCancel);

    auto pLayout = make_parented<QVBoxLayout>(this);
    pLayout->addWidget(pLabel.get());
    pLayout->addWidget(pSpacer.get());
    pLayout->addWidget(m_pProgressBar);
    pLayout->addWidget(m_pLabelCurrent);
    pLayout->addWidget(pCancel.get());
    setLayout(pLayout.get());
}

void LibraryScannerDlg::resetTaskCount() {
    m_tasksDone = 0;
    m_tasksTotal = 0;
    m_pProgressBar->reset();
}

void LibraryScannerDlg::addQueuedTasks(int num) {
    m_tasksTotal += num;
}

void LibraryScannerDlg::updateProgressBar() {
    // The scanner does three things:
    // 1) hash library directories and subdirectories
    // 2) analyze track and cover files in those directories
    // 3) analyze files outside library directories
    // For 1 and 2 we know the count in advance when ScannerTasks are queued and
    // store the total in m_tasksTotal.
    // For 3 we don't know the count before we are notified in slotUpdate() for
    // each, so at one point m_tasksDone can become bigger than m_tasksTotal.
    // To avoid the confusing situation of showing 100% (or more) on the progress
    // bar and label while the dialog is still updated at high rate,
    // we clamp to 99 %. 100 % will only be shown for a glimpse anyway before
    // the diaoge is closed so this is okay.
    //
    // Protect against division by zero. This may happen if we have no watched
    // library directories, i.e. only case 3)
    if (m_tasksTotal == 0) {
        if (m_showNoTasksQueuedWarning) {
            m_showNoTasksQueuedWarning = false;
            qWarning() << "LibraryScannerDlg::updateProgressBar(): received "
                          "scan progress update while we don't have any tasks queued.";
        }
        return;
    }

    int percent = static_cast<int>(m_tasksDone * 100.0 / m_tasksTotal);
    if (percent > 99) {
        return;
    }

    m_pProgressBar->setValue(percent);
}

void LibraryScannerDlg::slotUpdate(const QString& path) {
    // qDebug() << "LibraryScannerDlg slotUpdate" <<
    // m_timer.elapsed().formatMillisWithUnit() << path;
    if (!m_bCancelled && m_timer.elapsed() > mixxx::Duration::fromSeconds(2)) {
       setVisible(true);
    }

    m_tasksDone++;

    if (isVisible()) {
        updateProgressBar();
        m_pLabelCurrent->setText(tr("Scanning: ") + path);
    }
}

void LibraryScannerDlg::slotUpdateCover(const QString& path) {
    // qDebug() << "LibraryScannerDlg slotUpdate" << m_timer.elapsed() << path;
    if (!m_bCancelled && m_timer.elapsed() > mixxx::Duration::fromSeconds(2)) {
       setVisible(true);
    }

    m_tasksDone++;
    if (isVisible()) {
        updateProgressBar();
        const QString status = QStringLiteral("%1: %2").arg(
                tr("Scanning cover art (safe to cancel)"), path);
        m_pLabelCurrent->setText(status);
    }
}

void LibraryScannerDlg::slotCancel() {
    qDebug() << "Cancelling library scan...";
    m_bCancelled = true;
    emit scanCancelled();
    hide();
    resetTaskCount();
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
    resetTaskCount();
}
