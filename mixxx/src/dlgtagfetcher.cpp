#include <QTreeWidget>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>

#include "dlgtagfetcher.h"

DlgTagFetcher::DlgTagFetcher(QWidget *parent)
                    : QDialog(parent),
                      m_track(NULL) {
    setupUi(this);

    connect(btnApply, SIGNAL(clicked()),
            this, SLOT(apply()));
    connect(btnCancel, SIGNAL(clicked()),
            this, SLOT(cancel()));
    connect(btnPrev, SIGNAL(clicked()),
            this, SIGNAL(previous()));
    connect(btnNext, SIGNAL(clicked()),
            this, SIGNAL(next()));
    connect(results, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(ResultSelected()));

    // Resize columns, this can't be set in the ui file
    results->setColumnWidth(0, 50);  // Track column
    results->setColumnWidth(1, 50);  // Year column
    results->setColumnWidth(2, 160); // Title column
    results->setColumnWidth(3, 160); // Artist column
    results->setColumnWidth(4, 160); // Album column

    progressLabel->setText("");
}

DlgTagFetcher::~DlgTagFetcher() {
}

void DlgTagFetcher::init(const TrackPointer track) {
    results->clear();
    m_track = track;
    m_data = Data();
    UpdateStack();
}

void DlgTagFetcher::apply() {
    int resultIndex = m_data.m_selectedResult;
    if (resultIndex > -1) {
        m_track->setAlbum(m_data.m_results[resultIndex]->getAlbum());
        m_track->setArtist(m_data.m_results[resultIndex]->getArtist());
        m_track->setTitle(m_data.m_results[resultIndex]->getTitle());
        m_track->setYear(m_data.m_results[resultIndex]->getYear());
        m_track->setTrackNumber(m_data.m_results[resultIndex]->getTrackNumber());
        m_track.clear();
    }
    emit(finished());
    accept();
}

void DlgTagFetcher::cancel() {
    emit(finished());
    reject();
}

void DlgTagFetcher::FetchTagProgress(QString text) {
    loading->setText(text);
}

void DlgTagFetcher::FetchTagFinished(const TrackPointer track,
                                     const QList<TrackPointer>& tracks) {
    // check if the answer is for this track
    if (m_track->getLocation() != track->getLocation()) {
        return;
    }

    m_data.m_pending = false;
    m_data.m_results = tracks;
    // qDebug() << "number of results = " << tracks.size();
    UpdateStack();
}

void DlgTagFetcher::UpdateStack() {
    if (m_data.m_pending) {
        stack->setCurrentWidget(loading_page);
        return;
    } else if (m_data.m_results.isEmpty()) {
        stack->setCurrentWidget(error_page);
        return;
    }
    btnApply->setEnabled(true);
    stack->setCurrentWidget(results_page);

    // Clear tree widget
    results->clear();

    // Put the original tags at the top
    AddDivider(tr("Original tags"), results);
    AddTrack(m_track, -1, results);

    // Fill tree view with songs
    AddDivider(tr("Suggested tags"), results);

    int trackIndex = 0;
    foreach (const TrackPointer track, m_data.m_results) {
        AddTrack(track, trackIndex++, results);
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

void DlgTagFetcher::AddTrack(const TrackPointer track, int resultIndex,
                             QTreeWidget* parent) const {
    QStringList values;
    values << track->getTrackNumber() << track->getYear() << track->getTitle()
           << track->getArtist() << track->getAlbum();

    QTreeWidgetItem* item = new QTreeWidgetItem(parent, values);
    item->setData(0, Qt::UserRole, resultIndex);
    item->setData(0, Qt::TextAlignmentRole, Qt::AlignRight);
}

void DlgTagFetcher::AddDivider(const QString& text, QTreeWidget* parent) const {
    QTreeWidgetItem* item = new QTreeWidgetItem(parent);
    item->setFirstColumnSpanned(true);
    item->setText(0, text);
    item->setFlags(Qt::NoItemFlags);
    item->setForeground(0, palette().color(QPalette::Disabled, QPalette::Text));

    QFont bold_font(font());
    bold_font.setBold(true);
    item->setFont(0, bold_font);
}

void DlgTagFetcher::ResultSelected() {
  if (!results->currentItem())
    return;

  const int resultIndex = results->currentItem()->data(0, Qt::UserRole).toInt();
  m_data.m_selectedResult = resultIndex;
}
