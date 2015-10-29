#include <QPainter>
#include <QPushButton>

#include "library/previewbuttondelegate.h"
#include "library/trackmodel.h"
#include "playerinfo.h"
#include "playermanager.h"
#include "trackinfoobject.h"
#include "controlobjectslave.h"

PreviewButtonDelegate::PreviewButtonDelegate(QObject *parent, int column)
        : QStyledItemDelegate(parent),
          m_pTableView(NULL),
          m_pButton(NULL),
          m_isOneCellInEditMode(false),
          m_column(column) {
    m_pPreviewDeckPlay = new ControlObjectSlave(
            PlayerManager::groupForPreviewDeck(0), "play", this);
    m_pPreviewDeckPlay->connectValueChanged(SLOT(previewDeckPlayChanged(double)));

    m_pCueGotoAndPlay = new ControlObjectSlave(
            PlayerManager::groupForPreviewDeck(0), "cue_gotoandplay", this);

    // This assumes that the parent is wtracktableview
    connect(this, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)),
            parent, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)));

    if (QTableView *tableView = qobject_cast<QTableView*>(parent)) {
        m_pTableView = tableView;
        m_pButton = new QPushButton("", m_pTableView);
        m_pButton->setObjectName("LibraryPreviewButton");
        m_pButton->setCheckable(true);
        m_pButton->setChecked(false);
        m_pButton->hide();
        connect(m_pTableView, SIGNAL(entered(QModelIndex)),
                this, SLOT(cellEntered(QModelIndex)));
    }
}

PreviewButtonDelegate::~PreviewButtonDelegate() {
}

QWidget* PreviewButtonDelegate::createEditor(QWidget *parent,
                                             const QStyleOptionViewItem &option,
                                             const QModelIndex &index) const {
    Q_UNUSED(option);
    QPushButton* btn = new QPushButton(parent);
    btn->setObjectName("LibraryPreviewButton");
    btn->setCheckable(true);
    bool playing = m_pPreviewDeckPlay->toBool();
    // Check-state is whether the track is loaded (index.data()) and whether
    // it's playing.
    btn->setChecked(index.data().toBool() && playing);
    connect(btn, SIGNAL(clicked()),
            this, SLOT(buttonClicked()));
    connect(this, SIGNAL(buttonSetChecked(bool)),
            btn, SLOT(setChecked(bool)));
    return btn;
}

void PreviewButtonDelegate::setEditorData(QWidget *editor,
                                          const QModelIndex &index) const {
    Q_UNUSED(editor);
    Q_UNUSED(index);
}

void PreviewButtonDelegate::setModelData(QWidget *editor,
                                         QAbstractItemModel *model,
                                         const QModelIndex &index) const {
    Q_UNUSED(editor);
    Q_UNUSED(model);
    Q_UNUSED(index);
}

void PreviewButtonDelegate::paint(QPainter *painter,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const {
    // Let the editor paint in this case
    if (index == m_currentEditedCellIndex) {
        return;
    }

    if (!m_pButton) {
        return;
    }

    m_pButton->setGeometry(option.rect);
    bool playing = m_pPreviewDeckPlay->toBool();
    // Check-state is whether the track is loaded (index.data()) and whether
    // it's playing.
    m_pButton->setChecked(index.data().toBool() && playing);

    if (option.state == QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.base());
    }

    painter->save();
    // Render button at the desired position
    painter->translate(option.rect.topLeft());
    m_pButton->render(painter);
    painter->restore();
}

void PreviewButtonDelegate::updateEditorGeometry(QWidget *editor,
                                                 const QStyleOptionViewItem &option,
                                                 const QModelIndex &index) const {
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

QSize PreviewButtonDelegate::sizeHint(const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    if (!m_pButton) {
        return QSize();
    }
    return m_pButton->sizeHint();
}

void PreviewButtonDelegate::cellEntered(const QModelIndex &index) {
    if (!m_pTableView) {
        return;
    }
    // this slot is called if the mouse pointer enters ANY cell on
    // the QTableView but the code should only be executed on a button
    if (index.column() == m_column) {
        if (m_isOneCellInEditMode) {
            m_pTableView->closePersistentEditor(m_currentEditedCellIndex);
        }
        m_pTableView->openPersistentEditor(index);
        m_isOneCellInEditMode = true;
        m_currentEditedCellIndex = index;
    } else if (m_isOneCellInEditMode) { // close editor if the mouse leaves the button
        m_isOneCellInEditMode = false;
        m_pTableView->closePersistentEditor(m_currentEditedCellIndex);
        m_currentEditedCellIndex = QModelIndex();
    }
}

void PreviewButtonDelegate::buttonClicked() {
    if (!m_pTableView) {
        return;
    }

    TrackModel *pTrackModel = dynamic_cast<TrackModel*>(m_pTableView->model());
    if (!pTrackModel) {
        return;
    }

    QString group = PlayerManager::groupForPreviewDeck(0);
    TrackPointer pOldTrack = PlayerInfo::instance().getTrackInfo(group);
    bool playing = m_pPreviewDeckPlay->toBool();

    TrackPointer pTrack = pTrackModel->getTrack(m_currentEditedCellIndex);
    if (pTrack && pTrack != pOldTrack) {
        emit(loadTrackToPlayer(pTrack, group, true));
    } else if (pTrack == pOldTrack && !playing) {
        // Since the Preview deck might be hidden
        // Starting at cue is a predictable behavior
        m_pCueGotoAndPlay->set(1.0);
    } else {
        m_pPreviewDeckPlay->set(0.0);

    }
}

void PreviewButtonDelegate::previewDeckPlayChanged(double v) {
    m_pTableView->update();
    if (m_isOneCellInEditMode) {
        TrackModel *pTrackModel = dynamic_cast<TrackModel*>(m_pTableView->model());
        if (!pTrackModel) {
            return;
        }
        QString group = PlayerManager::groupForPreviewDeck(0);
        TrackPointer pPreviewTrack = PlayerInfo::instance().getTrackInfo(group);
        TrackPointer pTrack = pTrackModel->getTrack(m_currentEditedCellIndex);
        if (pTrack && pTrack == pPreviewTrack) {
            emit(buttonSetChecked(v > 0.0));
        }
    }
}
