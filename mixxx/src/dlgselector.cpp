#include <QSqlTableModel>
#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "library/selectorlibrarytablemodel.h"
#include "transposeproxymodel.h"
#include "widget/wselectorcratestableview.h"
#include "widget/wselectorlibrarytableview.h"
#include "library/trackcollection.h"
#include "dlgselector.h"


DlgSelector::DlgSelector(QWidget* parent,
                       ConfigObject<ConfigValue>* pConfig,
                       TrackCollection* pTrackCollection)
        : QWidget(parent), Ui::DlgSelector(),
          m_pConfig(pConfig),
          m_pTrackCollection(pTrackCollection) {
    setupUi(this);
    m_songsButtonGroup.addButton(radioButtonGenre);
    m_songsButtonGroup.addButton(radioButtonBpm);

    m_pSelectorLibraryTableView = new WSelectorLibraryTableView(this, pConfig, pTrackCollection,
                                                              ConfigKey(), ConfigKey());
    connect(m_pSelectorLibraryTableView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(m_pSelectorLibraryTableView, SIGNAL(loadTrackToPlayer(TrackPointer, QString)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString)));

    QBoxLayout* box = dynamic_cast<QBoxLayout*>(layout());
    Q_ASSERT(box); //Assumes the form layout is a QVBox/QHBoxLayout!
    box->removeWidget(m_pTrackTablePlaceholder);
    m_pTrackTablePlaceholder->hide();
    box->insertWidget(1, m_pSelectorLibraryTableView);

    m_pSelectorLibraryTableModel =  new SelectorLibraryTableModel(this, pTrackCollection);
    m_pSelectorLibraryTableView->loadTrackModel(m_pSelectorLibraryTableModel);

/*
    m_pCrateView = new CrateView(this, pTrackCollection);

    m_pSelectorCratesTableView = new WSelectorCratesTableView(this, pTrackCollection);
    box = dynamic_cast<QBoxLayout*>(horizontalLayoutCrates);
    Q_ASSERT(box); //Assumes the form layout is a QVBox/QHBoxLayout!
    box->removeWidget(m_pCratesViewPlaceholder);
    m_pCratesViewPlaceholder->hide();
    //box->insertWidget(1, m_pSelectorCratesTableView);
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
    m_pSelectorCratesTableView->setModel(m_pCratesTableModel);
*/

    connect(radioButtonGenre, SIGNAL(clicked()),
            this,  SLOT(filterByGenre()));
    connect(radioButtonBpm, SIGNAL(clicked()),
            this,  SLOT(filterByBpm()));

    radioButtonGenre->click();

    connect(m_pSelectorLibraryTableView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection&)),
            this,
            SLOT(tableSelectionChanged(const QItemSelection &, const QItemSelection&)));
}

DlgSelector::~DlgSelector() {
}

void DlgSelector::onShow()
{
    //Refresh crates
    //m_pCratesTableModel->select();
}


void DlgSelector::setup(QDomNode node)
{

}

void DlgSelector::onSearchStarting()
{
}

void DlgSelector::onSearchCleared()
{
}

void DlgSelector::onSearch(const QString& text)
{
    m_pSelectorLibraryTableModel->search(text);
}

void DlgSelector::loadSelectedTrack() {
    m_pSelectorLibraryTableView->loadSelectedTrack();
}

void DlgSelector::loadSelectedTrackToGroup(QString group) {
    m_pSelectorLibraryTableView->loadSelectedTrackToGroup(group);
}

void DlgSelector::moveSelection(int delta) {
    m_pSelectorLibraryTableView->moveSelection(delta);
}

void DlgSelector::tableSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
}

void DlgSelector::selectAll() {
    m_pSelectorLibraryTableView->selectAll();
}

void DlgSelector::filterByGenre()
{
    int datetimeColumn = m_pSelectorLibraryTableModel->fieldIndex(LIBRARYTABLE_DATETIMEADDED);
    // Don't tell the TableView to sortByColumn() because this generates excess
    // select()'s. Use setSort() on the model, and it will take effect when
    // showRecentSongs() select()'s.
    m_pSelectorLibraryTableModel->setSort(datetimeColumn, Qt::DescendingOrder);
    m_pSelectorLibraryTableModel->filterByGenre();
}

void DlgSelector::filterByBpm()
{
    m_pSelectorLibraryTableModel->filterByBpm();
}

void DlgSelector::installEventFilter(QObject* pFilter) {
    QWidget::installEventFilter(pFilter);
    m_pSelectorLibraryTableView->installEventFilter(pFilter);
}
