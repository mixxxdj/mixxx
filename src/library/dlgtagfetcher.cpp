#include <QTreeWidget>
#include <QtDebug>

#include "library/dlgtagfetcher.h"

DlgTagFetcher::DlgTagFetcher(QWidget *parent)
        : QDialog(parent),
          m_track(NULL),
          m_TagFetcher(parent),
          m_networkError(NOERROR) {
    init();
}

DlgTagFetcher::~DlgTagFetcher() {
}

void DlgTagFetcher::init() {
    setupUi(this);

    connect(btnApply, SIGNAL(clicked()),
            this, SLOT(apply()));
    connect(btnQuit, SIGNAL(clicked()),
            this, SLOT(quit()));
    connect(btnPrev, SIGNAL(clicked()),
            this, SIGNAL(previous()));
    connect(btnNext, SIGNAL(clicked()),
            this, SIGNAL(next()));
    connect(results, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(resultSelected()));

    connect(&m_TagFetcher, SIGNAL(resultAvailable(const TrackPointer,const QList<TrackPointer>&)),
            this, SLOT(fetchTagFinished(const TrackPointer,const QList<TrackPointer>&)));
    connect(&m_TagFetcher, SIGNAL(fetchProgress(QString)),
            this, SLOT(fetchTagProgress(QString)));
    connect(&m_TagFetcher, SIGNAL(networkError(int, QString)),
            this, SLOT(slotNetworkError(int, QString)));

    // Resize columns, this can't be set in the ui file
    results->setColumnWidth(0, 50);  // Track column
    results->setColumnWidth(1, 50);  // Year column
    results->setColumnWidth(2, 160); // Title column
    results->setColumnWidth(3, 160); // Artist column
    results->setColumnWidth(4, 160); // Album column
}

void DlgTagFetcher::loadTrack(const TrackPointer track) {
    if (track == NULL) {
        return;
    }
    results->clear();
    m_track = track;
    m_data = Data();
    m_networkError = NOERROR;
    m_TagFetcher.startFetch(m_track);

    disconnect(this, SLOT(updateTrackMetadata(TrackInfoObject*)));
    connect(track.data(), SIGNAL(changed(TrackInfoObject*)),
            this, SLOT(updateTrackMetadata(TrackInfoObject*)));

    updateStack();
}

void DlgTagFetcher::updateTrackMetadata(TrackInfoObject* pTIO) {
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
    m_TagFetcher.cancel();
    close();
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

void DlgTagFetcher::slotNetworkError(int errorCode, QString app) {
    m_networkError = errorCode==0 ?  FTWERROR : HTTPERROR;
    m_data.m_pending = false;
    QString httpStatusMessage = tr("HTTP Status: %1");
    httpStatus->setText(httpStatusMessage.arg(errorCode));
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
    } else if (m_networkError == HTTPERROR) {
        stack->setCurrentWidget(networkError_page);
        return;
    } else if (m_networkError == FTWERROR) {
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
