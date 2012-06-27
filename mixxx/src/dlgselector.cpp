#include <QSqlTableModel>
#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "library/selector/selectorlibrarytablemodel.h"
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
            this, SLOT(filterByGenre()));
    connect(checkBoxBpm, SIGNAL(clicked()),
            this, SLOT(filterByBpm()));
    connect(checkBoxYear, SIGNAL(clicked()),
            this, SLOT(filterByYear()));
    connect(checkBoxRating, SIGNAL(clicked()),
            this, SLOT(filterByRating()));
    connect(checkBoxKey, SIGNAL(clicked()),
            this, SLOT(filterByKey()));
    connect(checkBoxKey4th, SIGNAL(clicked()),
            this, SLOT(filterByKey4th()));
    connect(checkBoxKey5th, SIGNAL(clicked()),
            this, SLOT(filterByKey5th()));
    connect(checkBoxKeyRelative, SIGNAL(clicked()),
            this, SLOT(filterByKeyRelative()));
    connect(horizontalSliderBpmRange, SIGNAL(valueChanged(int)),
            this, SLOT(spinBoxBpmRangeChanged(int)));
    connect(m_pSelectorLibraryTableView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection&)),
            this, SLOT(tableSelectionChanged(const QItemSelection &, const QItemSelection&)));

    connect(m_pSelectorLibraryTableModel,
            SIGNAL(doSearch(const QItemSelection &, const QItemSelection&)),
            this,
            SLOT(tableSelectionChanged(const QItemSelection &, const QItemSelection&)));

    connect(m_pSelectorLibraryTableModel, SIGNAL(filtersChanged()),
            this, SLOT(slotFiltersChanged()));
    // Getting info on current decks playing etc
    connect(m_pSelectorLibraryTableModel, SIGNAL(currentTrackInfoChanged()),
           this, SLOT(slotCurrentTrackInfoChanged()));
            

}

DlgSelector::~DlgSelector() {
}

void DlgSelector::onShow()
{
    qDebug() << "DlgSelector::onShow()";
    m_pSelectorLibraryTableModel->active(true);
    slotCurrentTrackInfoChanged();
}

void DlgSelector::onHide()
{
    qDebug() << "DlgSelector::onHide()";
    m_pSelectorLibraryTableModel->active(false);
}

void DlgSelector::setup(QDomNode node)
{
    qDebug() << "DlgSelector::setup()";
}

void DlgSelector::onSearchStarting()
{
}

void DlgSelector::onSearchCleared()
{
}

void DlgSelector::slotFiltersChanged() {
    int count = m_pSelectorLibraryTableModel->rowCount();
    QString pluralize = ((count > 1) ? QString("s") : QString(""));
    QString labelMatchText = QString(tr("%1 Track%2 Found ")).arg(count).arg(pluralize);
    labelMatchCount->setText(labelMatchText);
}

void DlgSelector::slotCurrentTrackInfoChanged() {
    // check which filters to activate
    checkBoxGenre->setEnabled(m_pSelectorLibraryTableModel->currentTrackGenreExists());
    checkBoxBpm->setEnabled(m_pSelectorLibraryTableModel->currentTrackBpmExists());
    horizontalSliderBpmRange->setEnabled(m_pSelectorLibraryTableModel->currentTrackBpmExists());
    checkBoxYear->setEnabled(m_pSelectorLibraryTableModel->currentTrackYearExists());
    checkBoxRating->setEnabled(m_pSelectorLibraryTableModel->currentTrackRatingExists());
    checkBoxKey->setEnabled(m_pSelectorLibraryTableModel->currentTrackKeyExists());
    checkBoxKey4th->setEnabled(m_pSelectorLibraryTableModel->currentTrackKeyExists());
    checkBoxKey5th->setEnabled(m_pSelectorLibraryTableModel->currentTrackKeyExists());
    checkBoxKeyRelative->setEnabled(m_pSelectorLibraryTableModel->currentTrackKeyExists());
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

void DlgSelector::tableSelectionChanged(const QItemSelection& selected, 
	const QItemSelection& deselected)
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

void DlgSelector::filterByKey4th()
{
    m_pSelectorLibraryTableModel->filterByKey4th(checkBoxKey4th->isChecked());
}

void DlgSelector::filterByKey5th()
{
    m_pSelectorLibraryTableModel->filterByKey5th(checkBoxKey5th->isChecked());
}

void DlgSelector::filterByKeyRelative()
{
    m_pSelectorLibraryTableModel->filterByKeyRelative(checkBoxKeyRelative->isChecked());
}

void DlgSelector::installEventFilter(QObject* pFilter) {
    QWidget::installEventFilter(pFilter);
    m_pSelectorLibraryTableView->installEventFilter(pFilter);
}
