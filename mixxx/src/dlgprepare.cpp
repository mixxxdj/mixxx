#include <QSqlTableModel>
#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "library/preparelibrarytablemodel.h"
#include "transposeproxymodel.h"
#include "widget/wpreparecratestableview.h"
#include "widget/wpreparelibrarytableview.h"
#include "widget/crateview.h"
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

    m_pPrepareLibraryTableView = new WPrepareLibraryTableView(this, pConfig, 
                                                            ConfigKey(), ConfigKey());
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

void DlgPrepare::setup(QDomNode node)
{

    QPalette pal = palette();

    // Row colors
    if (!WWidget::selectNode(node, "BgColorRowEven").isNull() &&
        !WWidget::selectNode(node, "BgColorRowUneven").isNull()) {
        QColor r1;
        r1.setNamedColor(WWidget::selectNodeQString(node, "BgColorRowEven"));
        r1 = WSkinColor::getCorrectColor(r1);
        QColor r2;
        r2.setNamedColor(WWidget::selectNodeQString(node, "BgColorRowUneven"));
        r2 = WSkinColor::getCorrectColor(r2);

        // For now make text the inverse of the background so it's readable In
        // the future this should be configurable from the skin with this as the
        // fallback option
        QColor text(255 - r1.red(), 255 - r1.green(), 255 - r1.blue());

        //setAlternatingRowColors ( true );
        
        QColor fgColor;
        fgColor.setNamedColor(WWidget::selectNodeQString(node, "FgColor"));
        fgColor = WSkinColor::getCorrectColor(fgColor);

        pal.setColor(QPalette::Base, r1);
        pal.setColor(QPalette::AlternateBase, r2);
        pal.setColor(QPalette::Text, text);
        pal.setColor(QPalette::WindowText, fgColor);

    }

    setPalette(pal);
    
    radioButtonRecentlyAdded->setPalette(pal);
    radioButtonAllSongs->setPalette(pal);
    pushButtonSelectAll->setPalette(pal);
    pushButtonAnalyze->setPalette(pal);
    m_pPrepareLibraryTableView->setPalette(pal);
    //m_pPrepareCratesTableView->setPalette(pal);
    //m_pCrateView->setPalette(pal);
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
    //Screw you QSqlTableModel.
    while(m_pPrepareLibraryTableModel->canFetchMore()) {
        m_pPrepareLibraryTableModel->fetchMore();
    }
    m_pPrepareLibraryTableView->selectAll();
}

void DlgPrepare::analyze()
{
    if (m_pAnalyserQueue != NULL)
    {
        stopAnalysis();
        return;
    }
    else
    {
        m_pAnalyserQueue = AnalyserQueue::createPrepareViewAnalyserQueue(m_pConfig);
        
        connect(m_pAnalyserQueue, SIGNAL(trackProgress(TrackInfoObject*, int)),
                this, SLOT(trackAnalysisProgress(TrackInfoObject*, int)));
        connect(m_pAnalyserQueue, SIGNAL(trackFinished(TrackInfoObject*)),
                this, SLOT(trackAnalysisFinished(TrackInfoObject*)));

        QModelIndex selectedIndex;
        m_indexesBeingAnalyzed = m_pPrepareLibraryTableView->selectionModel()->selectedRows();
        foreach(selectedIndex, m_indexesBeingAnalyzed) 
        {
            TrackInfoObject* tio = m_pPrepareLibraryTableModel->getTrack(selectedIndex);
            qDebug() << "Queueing track" << tio->getLocation();
            m_pAnalyserQueue->queueAnalyseTrack(tio);
        }
        pushButtonAnalyze->setText("Stop Analysis");
    }
}

void DlgPrepare::trackAnalysisFinished(TrackInfoObject* tio)
{
    qDebug() << "Analysis finished!";
    m_pTrackCollection->getTrackDAO().updateTrackInDatabase(tio);
    delete tio;

    //If the analyser has already been deleted by the time we get this signal
    //or there are no tracks in it when we do get the signal, then say we're done.
    if (!m_pAnalyserQueue || m_pAnalyserQueue->numQueuedTracks() == 0)
    {
        stopAnalysis();
    }
}

void DlgPrepare::trackAnalysisProgress(TrackInfoObject* tio, int progress)
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

    //Tell the model to notify the view that the track data has potentially changed.
    m_pPrepareLibraryTableModel->updateTracks(m_indexesBeingAnalyzed);
    //XXX: ^^ This totally doesn't work. :(
}

void DlgPrepare::showRecentSongs()
{
    int datetimeColumn = m_pPrepareLibraryTableModel->fieldIndex(LIBRARYTABLE_DATETIMEADDED);
    m_pPrepareLibraryTableView->sortByColumn(datetimeColumn, Qt::DescendingOrder);
    m_pPrepareLibraryTableModel->showRecentSongs();
}

void DlgPrepare::showAllSongs()
{
    m_pPrepareLibraryTableModel->showAllSongs();
}
