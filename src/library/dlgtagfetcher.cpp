#include <QTreeWidget>
#include <QtDebug>

#include "library/dlgtagfetcher.h"

DlgTagFetcher::DlgTagFetcher(QWidget *parent)
        : QDialog(parent),
          m_tagFetcher(parent),
          m_networkResult(NetworkResult::Ok) {
    init();
}

void DlgTagFetcher::init() {
    setupUi(this);

    connect(btnApply, &QPushButton::clicked,
            this, &DlgTagFetcher::apply);
    connect(btnQuit, &QPushButton::clicked,
            this, &DlgTagFetcher::quit);
    connect(btnPrev, &QPushButton::clicked,
            this, &DlgTagFetcher::previous);
    connect(btnNext, &QPushButton::clicked,
            this, &DlgTagFetcher::next);
    connect(results, &QTreeWidget::currentItemChanged,
            this, &DlgTagFetcher::resultSelected);

    connect(&m_tagFetcher, &TagFetcher::resultAvailable,
            this, &DlgTagFetcher::fetchTagFinished);
    connect(&m_tagFetcher, &TagFetcher::fetchProgress,
            this, &DlgTagFetcher::fetchTagProgress);
    connect(&m_tagFetcher, &TagFetcher::networkError,
            this, &DlgTagFetcher::slotNetworkResult);

    // Resize columns, this can't be set in the ui file
    results->setColumnWidth(0, 50);  // Track column
    results->setColumnWidth(1, 50);  // Year column
    results->setColumnWidth(2, 160); // Title column
    results->setColumnWidth(3, 160); // Artist column
    results->setColumnWidth(4, 160); // Album column
}

void DlgTagFetcher::loadTrack(const TrackPointer& track) {
    if (track == NULL) {
        return;
    }
    results->clear();
    disconnect(track.get(), &Track::changed,
            this, &DlgTagFetcher::updateTrackMetadata);

    m_track = track;
    m_data = Data();
    m_networkResult = NetworkResult::Ok;
    m_tagFetcher.startFetch(m_track);

    connect(track.get(), &Track::changed,
            this, &DlgTagFetcher::updateTrackMetadata);

    updateStack();
}

void DlgTagFetcher::updateTrackMetadata(Track* pTIO) {
    Q_UNUSED(pTIO);
    updateStack();
}

void DlgTagFetcher::apply() {
    int resultIndex = m_data.m_selectedResult;
    if (resultIndex > -1) {
        if (!m_data.m_results[resultIndex]->getAlbum().isEmpty()) {
            m_track->setAlbum(m_data.m_results[resultIndex]->getAlbum());
        }
        if (!m_data.m_results[resultIndex]->getArtist().isEmpty()) {
            m_track->setArtist(m_data.m_results[resultIndex]->getArtist());
        }
        if (!m_data.m_results[resultIndex]->getTitle().isEmpty()) {
            m_track->setTitle(m_data.m_results[resultIndex]->getTitle());
        }
        if (!m_data.m_results[resultIndex]->getYear().isEmpty() &&
             m_data.m_results[resultIndex]->getYear() != "0") {
            m_track->setYear(m_data.m_results[resultIndex]->getYear());
        }
        if (!m_data.m_results[resultIndex]->getTrackNumber().isEmpty() &&
             m_data.m_results[resultIndex]->getTrackNumber() != "0") {
            m_track->setTrackNumber(m_data.m_results[resultIndex]->getTrackNumber());
        }
    }
}

void DlgTagFetcher::quit() {
    m_tagFetcher.cancel();
    accept();
}

void DlgTagFetcher::fetchTagProgress(QString text) {
    QString status = tr("Status: %1");
    loadingStatus->setText(status.arg(text));
}

void DlgTagFetcher::fetchTagFinished(const TrackPointer track,
                                     const QList<TrackPointer>& tracks) {
    // check if the answer is for this track
    if (m_track->getLocation() != track->getLocation()) {
        return;
    }

    m_data.m_pending = false;
    m_data.m_results = tracks;
    // qDebug() << "number of results = " << tracks.size();
    updateStack();
}

void DlgTagFetcher::slotNetworkResult(int httpError, QString app, QString message, int code) {
    m_networkResult = httpError == 0 ?  NetworkResult::UnknownError : NetworkResult::HttpError;
    m_data.m_pending = false;
    QString strError = tr("HTTP Status: %1");
    QString strCode = tr("Code: %1");
    httpStatus->setText(strError.arg(httpError) + "\n" + strCode.arg(code) + "\n" + message);
    QString unknownError = tr("Mixxx can't connect to %1 for an unknown reason.");
    cantConnectMessage->setText(unknownError.arg(app));
    QString cantConnect = tr("Mixxx can't connect to %1.");
    cantConnectHttp->setText(cantConnect.arg(app));
    updateStack();
}

void DlgTagFetcher::updateStack() {
    if (m_data.m_pending) {
        stack->setCurrentWidget(loading_page);
        return;
    } else if (m_networkResult == NetworkResult::HttpError) {
        stack->setCurrentWidget(networkError_page);
        return;
    } else if (m_networkResult == NetworkResult::UnknownError) {
        stack->setCurrentWidget(generalnetworkError_page);
        return;
    } else if (m_data.m_results.isEmpty()) {
        stack->setCurrentWidget(error_page);
        return;
    }
    btnApply->setEnabled(true);
    stack->setCurrentWidget(results_page);

    results->clear();

    addDivider(tr("Original tags"), results);
    addTrack(m_track, -1, results);

    addDivider(tr("Suggested tags"), results);

    int trackIndex = 0;
    foreach (const TrackPointer track, m_data.m_results) {
        addTrack(track, trackIndex++, results);
    }

    // Find the item that was selected last time
    for (int i=0 ; i<results->model()->rowCount() ; ++i) {
        const QModelIndex index = results->model()->index(i, 0);
        const QVariant id = index.data(Qt::UserRole);
        if (!id.isNull() && id.toInt() == m_data.m_selectedResult) {
            results->setCurrentIndex(index);
            break;
        }
    }
}

void DlgTagFetcher::addTrack(const TrackPointer track, int resultIndex,
                             QTreeWidget* parent) const {
    QStringList values;
    values << track->getTrackNumber() << track->getYear() << track->getTitle()
           << track->getArtist() << track->getAlbum();

    QTreeWidgetItem* item = new QTreeWidgetItem(parent, values);
    item->setData(0, Qt::UserRole, resultIndex);
    item->setData(0, Qt::TextAlignmentRole, Qt::AlignRight);
}

void DlgTagFetcher::addDivider(const QString& text, QTreeWidget* parent) const {
    QTreeWidgetItem* item = new QTreeWidgetItem(parent);
    item->setFirstColumnSpanned(true);
    item->setText(0, text);
    item->setFlags(Qt::NoItemFlags);
    item->setForeground(0, palette().color(QPalette::Disabled, QPalette::Text));

    QFont bold_font(font());
    bold_font.setBold(true);
    item->setFont(0, bold_font);
}

void DlgTagFetcher::resultSelected() {
  if (!results->currentItem())
    return;

  const int resultIndex = results->currentItem()->data(0, Qt::UserRole).toInt();
  m_data.m_selectedResult = resultIndex;
}
