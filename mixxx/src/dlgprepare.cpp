#include <QSqlTableModel>
#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "library/preparelibrarytablemodel.h"
#include "transposeproxymodel.h"
#include "widget/wpreparecratestableview.h"
#include "widget/wpreparelibrarytableview.h"
#include "analyserqueue.h"
#include "library/trackcollection.h"
#include "dlgprepare.h"


DlgPrepare::DlgPrepare(QWidget* parent, ConfigObject<ConfigValue>* pConfig, TrackCollection* pTrackCollection) : QWidget(parent), Ui::DlgPrepare()
{
    setupUi(this);

    m_pConfig = pConfig;
    m_pTrackCollection = pTrackCollection;
    m_pAnalyserQueue = NULL;

    m_songsButtonGroup.addButton(radioButtonRecentlyAdded);
    m_songsButtonGroup.addButton(radioButtonAllSongs);

    m_pPrepareLibraryTableView = new WPrepareLibraryTableView(this, pConfig, pTrackCollection,
                                                            ConfigKey(), ConfigKey());
    connect(m_pPrepareLibraryTableView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(m_pPrepareLibraryTableView, SIGNAL(loadTrackToPlayer(TrackPointer, QString)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString)));

    QBoxLayout* box = dynamic_cast<QBoxLayout*>(layout());
    Q_ASSERT(box); //Assumes the form layout is a QVBox/QHBoxLayout!
    box->removeWidget(m_pTrackTablePlaceholder);
    m_pTrackTablePlaceholder->hide();
    box->insertWidget(1, m_pPrepareLibraryTableView);

    m_pPrepareLibraryTableModel =  new PrepareLibraryTableModel(this, pTrackCollection);
    m_pPrepareLibraryTableView->loadTrackModel(m_pPrepareLibraryTableModel);

/*
    m_pCrateView = new CrateView(this, pTrackCollection);

    m_pPrepareCratesTableView = new WPrepareCratesTableView(this, pTrackCollection);
    box = dynamic_cast<QBoxLayout*>(horizontalLayoutCrates);
    Q_ASSERT(box); //Assumes the form layout is a QVBox/QHBoxLayout!
    box->removeWidget(m_pCratesViewPlaceholder);
    m_pCratesViewPlaceholder->hide();
    //box->insertWidget(1, m_pPrepareCratesTableView);
    box->insertWidget(1, m_pCrateView);
    m_pCrateView->show();

    m_pCratesTableModel = new QSqlTableModel(this);
    m_pCratesTableModel->setTable("crates");
    m_pCratesTableModel->removeColumn(m_pCratesTableModel->fieldIndex("id"));
    m_pCratesTableModel->removeColumn(m_pCratesTableModel->fieldIndex("show"));
    m_pCratesTableModel->removeColumn(m_pCratesTableModel->fieldIndex("count"));
    m_pCratesTableModel->setSort(m_pCratesTableModel->fieldIndex("name"),
                              Qt::AscendingOrder);
    m_pCratesTableModel->setFilter("show = 1");
    m_pCratesTableModel->select();
    TransposeProxyModel* transposeProxy = new TransposeProxyModel(this);
    transposeProxy->setSourceModel(m_pCratesTableModel);
    m_pPrepareCratesTableView->setModel(m_pCratesTableModel);
*/

    connect(radioButtonRecentlyAdded, SIGNAL(clicked()),
            this,  SLOT(showRecentSongs()));
    connect(radioButtonAllSongs, SIGNAL(clicked()),
            this,  SLOT(showAllSongs()));

    radioButtonRecentlyAdded->click();

    labelProgress->setText("");
    pushButtonAnalyze->setEnabled(false);
    connect(pushButtonAnalyze, SIGNAL(clicked()),
            this, SLOT(analyze()));

    connect(pushButtonSelectAll, SIGNAL(clicked()),
            this, SLOT(selectAll()));

    connect(m_pPrepareLibraryTableView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection&)),
            this,
            SLOT(tableSelectionChanged(const QItemSelection &, const QItemSelection&)));
}

DlgPrepare::~DlgPrepare()
{
    delete m_pAnalyserQueue;
}

void DlgPrepare::onShow()
{
    //Refresh crates
    //m_pCratesTableModel->select();
}

QWidget* DlgPrepare::getWidgetForMIDIControl()
{
    return m_pPrepareLibraryTableView;
}

void DlgPrepare::setup(QDomNode node)
{

}
void DlgPrepare::onSearchStarting()
{
}
void DlgPrepare::onSearchCleared()
{
}
void DlgPrepare::onSearch(const QString& text)
{
    m_pPrepareLibraryTableModel->search(text);
}

void DlgPrepare::tableSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    if (selected == QItemSelection()) //Empty selection
        pushButtonAnalyze->setEnabled(false);
    else
        pushButtonAnalyze->setEnabled(true);
}

void DlgPrepare::selectAll()
{
    m_pPrepareLibraryTableView->selectAll();
}

void DlgPrepare::analyze()
{
    //Save the old BPM detection prefs setting (on or off)
    m_iOldBpmEnabled = m_pConfig->getValueString(ConfigKey("[BPM]","BPMDetectionEnabled")).toInt();
    //Force BPM detection to be on.
    m_pConfig->set(ConfigKey("[BPM]","BPMDetectionEnabled"), ConfigValue(1));
    //Note: this sucks... we should refactor the prefs/analyser to fix this hacky bit ^^^^.

    if (m_pAnalyserQueue != NULL)
    {
        stopAnalysis();
        return;
    }
    else
    {
        m_pAnalyserQueue = AnalyserQueue::createPrepareViewAnalyserQueue(m_pConfig);

        connect(m_pAnalyserQueue, SIGNAL(trackProgress(TrackPointer, int)),
                this, SLOT(trackAnalysisProgress(TrackPointer, int)));
        connect(m_pAnalyserQueue, SIGNAL(trackFinished(TrackPointer)),
                this, SLOT(trackAnalysisFinished(TrackPointer)));

        QModelIndex selectedIndex;
        m_indexesBeingAnalyzed = m_pPrepareLibraryTableView->selectionModel()->selectedRows();
        foreach(selectedIndex, m_indexesBeingAnalyzed)
        {
            TrackPointer tio = m_pPrepareLibraryTableModel->getTrack(selectedIndex);
            qDebug() << "Queueing track" << tio->getLocation();
            m_pAnalyserQueue->queueAnalyseTrack(tio);
        }
        pushButtonAnalyze->setText("Stop Analysis");
    }
}

void DlgPrepare::trackAnalysisFinished(TrackPointer tio)
{
    qDebug() << "Analysis finished on track:" << tio->getInfo();

    // TrackPointer auto-deletes once nobody is referencing it anymore.
    m_pTrackCollection->getTrackDAO().saveTrack(tio);



    //If the analyser has already been deleted by the time we get this signal
    //or there are no tracks in it when we do get the signal, then say we're done.
    if (!m_pAnalyserQueue || m_pAnalyserQueue->numQueuedTracks() == 0)
    {
        stopAnalysis();
    }
}

void DlgPrepare::trackAnalysisProgress(TrackPointer tio, int progress)
{
    QString text = "Analyzing";
    labelProgress->setText(QString("%1 %2\%").arg(text).arg(progress));
}

void DlgPrepare::stopAnalysis()
{
    //Stop analysis!
    if (m_pAnalyserQueue)
        m_pAnalyserQueue->stop();
    labelProgress->setText("");
    delete m_pAnalyserQueue;
    m_pAnalyserQueue = NULL;
    pushButtonAnalyze->setText("Analyze");

    //Restore old BPM detection setting for preferences...
    m_pConfig->set(ConfigKey("[BPM]","BPMDetectionEnabled"), ConfigValue(m_iOldBpmEnabled));

    //Tell the model to notify the view that the track data has potentially changed.
    m_pPrepareLibraryTableModel->updateTracks(m_indexesBeingAnalyzed);
    //XXX: ^^ This totally doesn't work. :(
}

void DlgPrepare::showRecentSongs()
{
    int datetimeColumn = m_pPrepareLibraryTableModel->fieldIndex(LIBRARYTABLE_DATETIMEADDED);
    // Don't tell the TableView to sortByColumn() because this generates excess
    // select()'s. Use setSort() on the model, and it will take effect when
    // showRecentSongs() select()'s.
    m_pPrepareLibraryTableModel->setSort(datetimeColumn, Qt::DescendingOrder);
    m_pPrepareLibraryTableModel->showRecentSongs();
}

void DlgPrepare::showAllSongs()
{
    m_pPrepareLibraryTableModel->showAllSongs();
}

void DlgPrepare::installEventFilter(QObject* pFilter) {
    QWidget::installEventFilter(pFilter);
    m_pPrepareLibraryTableView->installEventFilter(pFilter);
}
