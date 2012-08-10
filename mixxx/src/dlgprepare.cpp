#include <QSqlTableModel>
#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "library/preparelibrarytablemodel.h"
#include "transposeproxymodel.h"
#include "widget/wpreparecratestableview.h"
#include "widget/wpreparelibrarytableview.h"
#include "library/trackcollection.h"
#include "dlgprepare.h"


DlgPrepare::DlgPrepare(QWidget* parent,
                       ConfigObject<ConfigValue>* pConfig,
                       TrackCollection* pTrackCollection)
        : QWidget(parent),
          m_pConfig(pConfig),
          m_pTrackCollection(pTrackCollection),
          m_bAnalysisActive(false) {
    setupUi(this);
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

DlgPrepare::~DlgPrepare() {
}

void DlgPrepare::onShow()
{
    // Refresh table
    // There might be new tracks dropped to other views
    m_pPrepareLibraryTableModel->select();
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

void DlgPrepare::loadSelectedTrack() {
    m_pPrepareLibraryTableView->loadSelectedTrack();
}

void DlgPrepare::loadSelectedTrackToGroup(QString group) {
    m_pPrepareLibraryTableView->loadSelectedTrackToGroup(group);
}

void DlgPrepare::moveSelection(int delta) {
    m_pPrepareLibraryTableView->moveSelection(delta);
}

void DlgPrepare::tableSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    if (selected == QItemSelection()) //Empty selection
        pushButtonAnalyze->setEnabled(false);
    else
        pushButtonAnalyze->setEnabled(true);
}

void DlgPrepare::selectAll() {
    m_pPrepareLibraryTableView->selectAll();
}

void DlgPrepare::analyze() {
    //qDebug() << this << "analyze()";
    if (m_bAnalysisActive) {
        emit(stopAnalysis());
    } else {
        QList<int> trackIds;

        QModelIndexList selectedIndexes = m_pPrepareLibraryTableView->selectionModel()->selectedRows();
        foreach(QModelIndex selectedIndex, selectedIndexes) {
            bool ok;
            int trackId = selectedIndex.sibling(
                selectedIndex.row(),
                m_pPrepareLibraryTableModel->fieldIndex(LIBRARYTABLE_ID)).data().toInt(&ok);
            if (ok) {
                trackIds.append(trackId);
            }
        }
        emit(analyzeTracks(trackIds));
    }
}

void DlgPrepare::analysisActive(bool bActive) {
    qDebug() << this << "analysisActive" << bActive;
    m_bAnalysisActive = bActive;
    if (bActive) {
        pushButtonAnalyze->setEnabled(true);
        pushButtonAnalyze->setText(tr("Stop Analysis"));
    } else {
        pushButtonAnalyze->setText(tr("Analyze"));
        labelProgress->setText("");
    }
}

void DlgPrepare::trackAnalysisFinished(TrackPointer tio) {
    qDebug() << "Analysis finished on track:" << tio->getInfo();
}

void DlgPrepare::trackAnalysisProgress(TrackPointer tio, int progress) {
    if (m_bAnalysisActive) {
        QString text = tr("Analyzing %1%").arg(progress);
        labelProgress->setText(text);
    }
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
