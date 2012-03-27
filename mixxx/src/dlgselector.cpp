#include <QSqlTableModel>
#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "library/selectorlibrarytablemodel.h"
#include "transposeproxymodel.h"
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

    connect(checkBoxGenre, SIGNAL(clicked()),
            this,  SLOT(filterByGenre()));
    connect(checkBoxBpm, SIGNAL(clicked()),
            this,  SLOT(filterByBpm()));
    connect(checkBoxYear, SIGNAL(clicked()),
            this,  SLOT(filterByYear()));
    connect(checkBoxRating, SIGNAL(clicked()),
            this,  SLOT(filterByRating()));
    connect(checkBoxKey, SIGNAL(clicked()),
            this,  SLOT(filterByKey()));
    connect(checkBoxHarmonicKey, SIGNAL(clicked()),
            this,  SLOT(filterByHarmonicKey()));
    connect(horizontalSliderBpmRange, SIGNAL(valueChanged(int)),
            this,  SLOT(spinBoxBpmRangeChanged(int)));

    checkBoxGenre->click();

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
    m_pSelectorLibraryTableModel->filterByGenre(checkBoxGenre->isChecked());
}

void DlgSelector::filterByBpm()
{
    bool bpm = checkBoxBpm->isChecked();
    int range = horizontalSliderBpmRange->value();
    qDebug() << "Range " << range << " ";

    m_pSelectorLibraryTableModel->filterByBpm(bpm, range);
}

void DlgSelector::spinBoxBpmRangeChanged(int value)
{
    filterByBpm();
}

void DlgSelector::filterByYear()
{
    m_pSelectorLibraryTableModel->filterByYear(checkBoxYear->isChecked());
}

void DlgSelector::filterByRating()
{
    m_pSelectorLibraryTableModel->filterByRating(checkBoxRating->isChecked());
}

void DlgSelector::filterByKey()
{
    m_pSelectorLibraryTableModel->filterByKey(checkBoxKey->isChecked());
}

void DlgSelector::filterByHarmonicKey()
{
    m_pSelectorLibraryTableModel->filterByHarmonicKey(checkBoxHarmonicKey->isChecked());
}

void DlgSelector::installEventFilter(QObject* pFilter) {
    QWidget::installEventFilter(pFilter);
    m_pSelectorLibraryTableView->installEventFilter(pFilter);
}
