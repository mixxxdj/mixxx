#include <QSqlTableModel>

#include "dlgselector.h"

#include "library/selector/selectorlibrarytablemodel.h"
#include "library/trackcollection.h"
#include "widget/wskincolor.h"
#include "widget/wtracktableview.h"
#include "widget/wwidget.h"

DlgSelector::DlgSelector(QWidget* parent,
                         ConfigObject<ConfigValue>* pConfig,
                         TrackCollection* pTrackCollection,
                         MixxxKeyboard* pKeyboard)
        : QWidget(parent), Ui::DlgSelector(),
          m_pConfig(pConfig),
          m_pTrackCollection(pTrackCollection),
          m_pTrackTableView(
              new WTrackTableView(this, pConfig, m_pTrackCollection, true)) {
    setupUi(this);

    m_pTrackTableView->installEventFilter(pKeyboard);
    connect(m_pTrackTableView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(m_pTrackTableView, SIGNAL(loadTrackToPlayer(TrackPointer, QString)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString)));

    QBoxLayout* box = dynamic_cast<QBoxLayout*>(layout());
    Q_ASSERT(box); //Assumes the form layout is a QVBox/QHBoxLayout!
    box->removeWidget(m_pTrackTablePlaceholder);
    m_pTrackTablePlaceholder->hide();
    box->insertWidget(1, m_pTrackTableView);

    m_pSelectorLibraryTableModel =  new SelectorLibraryTableModel(this, pTrackCollection);
    m_pTrackTableView->loadTrackModel(m_pSelectorLibraryTableModel);

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
    connect(m_pSelectorLibraryTableModel, SIGNAL(filtersChanged()),
            this, SLOT(slotFiltersChanged()));
    // Getting info on current decks playing etc
    connect(m_pSelectorLibraryTableModel, SIGNAL(seedTrackInfoChanged()),
           this, SLOT(slotSeedTrackInfoChanged()));
            

}

DlgSelector::~DlgSelector() {
}

void DlgSelector::onShow()
{
    qDebug() << "DlgSelector::onShow()";
    m_pSelectorLibraryTableModel->active(true);
    slotSeedTrackInfoChanged();
}

void DlgSelector::onHide() {
    qDebug() << "DlgSelector::onHide()";
    m_pSelectorLibraryTableModel->active(false);
}

void DlgSelector::onSearch(const QString& text) {
    m_pSelectorLibraryTableModel->search(text);
}


void DlgSelector::slotFiltersChanged() {
    int count = m_pSelectorLibraryTableModel->rowCount();
    QString pluralize = ((count > 1 || count == 0) ? QString("s") : QString(""));
    QString labelMatchText = QString(tr("%1 Track%2 Found ")).arg(count).arg(pluralize);
    labelMatchCount->setText(labelMatchText);
}

void DlgSelector::slotSeedTrackInfoChanged() {
    QString seedTrackInfo = m_pSelectorLibraryTableModel->getSeedTrackInfo();
    QString labelSeedTrackText = QString(tr("Matches for: %1")).arg(seedTrackInfo);
    labelSeedTrackInfo->setText(labelSeedTrackText);

    // check which filters to activate
    checkBoxGenre->setEnabled(m_pSelectorLibraryTableModel->seedTrackGenreExists());
    checkBoxBpm->setEnabled(m_pSelectorLibraryTableModel->seedTrackBpmExists());
    horizontalSliderBpmRange->setEnabled(m_pSelectorLibraryTableModel->seedTrackBpmExists());
    checkBoxYear->setEnabled(m_pSelectorLibraryTableModel->seedTrackYearExists());
    checkBoxRating->setEnabled(m_pSelectorLibraryTableModel->seedTrackRatingExists());
    checkBoxKey->setEnabled(m_pSelectorLibraryTableModel->seedTrackKeyExists());
    checkBoxKey4th->setEnabled(m_pSelectorLibraryTableModel->seedTrackKeyExists());
    checkBoxKey5th->setEnabled(m_pSelectorLibraryTableModel->seedTrackKeyExists());
    checkBoxKeyRelative->setEnabled(m_pSelectorLibraryTableModel->seedTrackKeyExists());
}

void DlgSelector::setSeedTrack(TrackPointer pSeedTrack) {
    m_pSelectorLibraryTableModel->setSeedTrack(pSeedTrack);
}

void DlgSelector::loadSelectedTrack() {
    m_pTrackTableView->loadSelectedTrack();
}

void DlgSelector::loadSelectedTrackToGroup(QString group) {
    m_pTrackTableView->loadSelectedTrackToGroup(group, false);
}

void DlgSelector::moveSelection(int delta) {
    m_pTrackTableView->moveSelection(delta);
}

void DlgSelector::tableSelectionChanged(const QItemSelection& selected, 
    const QItemSelection& deselected) {
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
}

void DlgSelector::selectAll() {
    m_pTrackTableView->selectAll();
}

void DlgSelector::filterByGenre() {
    m_pSelectorLibraryTableModel->filterByGenre(checkBoxGenre->isChecked());
}

void DlgSelector::filterByBpm() {
    bool bpm = checkBoxBpm->isChecked();
    int range = horizontalSliderBpmRange->value();
    m_pSelectorLibraryTableModel->filterByBpm(bpm, range);
}

void DlgSelector::spinBoxBpmRangeChanged(int value) {
    Q_UNUSED(value);
    filterByBpm();
}

void DlgSelector::filterByYear() {
    m_pSelectorLibraryTableModel->filterByYear(checkBoxYear->isChecked());
}

void DlgSelector::filterByRating() {
    m_pSelectorLibraryTableModel->filterByRating(checkBoxRating->isChecked());
}

void DlgSelector::filterByKey() {
    m_pSelectorLibraryTableModel->filterByKey(checkBoxKey->isChecked());
}

void DlgSelector::filterByKey4th() {
    m_pSelectorLibraryTableModel->filterByKey4th(checkBoxKey4th->isChecked());
}

void DlgSelector::filterByKey5th() {
    m_pSelectorLibraryTableModel->filterByKey5th(checkBoxKey5th->isChecked());
}

void DlgSelector::filterByKeyRelative() {
    m_pSelectorLibraryTableModel->filterByKeyRelative(checkBoxKeyRelative->isChecked());
}

void DlgSelector::installEventFilter(QObject* pFilter) {
    QWidget::installEventFilter(pFilter);
    m_pTrackTableView->installEventFilter(pFilter);
}
