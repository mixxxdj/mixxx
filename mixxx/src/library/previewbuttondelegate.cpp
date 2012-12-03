#include <QPainter>
#include <QPushButton>

#include "library/previewbuttondelegate.h"
#include "library/trackmodel.h"
#include "playerinfo.h"
#include "playermanager.h"
#include "trackinfoobject.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"

PreviewButtonDelegate::PreviewButtonDelegate(QObject *parent, int column)
        : QStyledItemDelegate(parent) {
    if (QTableView *tableView = qobject_cast<QTableView*>(parent)) {
        m_pTableView = tableView;
        m_pButton = new QPushButton("", m_pTableView);
        m_pButton->setObjectName("LibraryPreviewButton");
        m_pButton->setIcon(QIcon(":/images/library/ic_library_preview_play.png"));
        m_pButton->hide();
        connect(m_pTableView, SIGNAL(entered(QModelIndex)),
                this, SLOT(cellEntered(QModelIndex)));

        // This assumes that the parent is wtracktableview
        connect(this, SIGNAL(loadTrackToPlayer(TrackPointer, QString)),
                parent, SIGNAL(loadTrackToPlayer(TrackPointer, QString)));

        m_isOneCellInEditMode = false;
        m_column = column;
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
    btn->setIcon(QIcon(":/images/library/ic_library_preview_play.png"));
    connect(btn, SIGNAL(clicked()),
            this, SLOT(buttonClicked()));
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
    Q_UNUSED(index);
    m_pButton->setGeometry(option.rect);
    if (option.state == QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.base());
    QPixmap map = QPixmap::grabWidget(m_pButton);
    painter->drawPixmap(option.rect.x(), option.rect.y(), map);
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
    return m_pButton->sizeHint();
}

void PreviewButtonDelegate::cellEntered(const QModelIndex &index) {
    //this slot is called if the mouse pointer enters ANY cell on
    //the QTableView but the code should only be executed on a button
    if (index.column() == m_column) {
        if (m_isOneCellInEditMode) {
            m_pTableView->closePersistentEditor(m_currentEditedCellIndex);
        }
        m_pTableView->openPersistentEditor(index);
        m_isOneCellInEditMode = true;
        m_currentEditedCellIndex = index;
    } else { // close editor if the mouse leaves the button
        if (m_isOneCellInEditMode) {
            m_isOneCellInEditMode = false;
            m_pTableView->closePersistentEditor(m_currentEditedCellIndex);
        }
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
    TrackPointer pOldTrack = PlayerInfo::Instance().getTrackInfo(group);

    ControlObjectThreadMain* playStatus = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(group, "play")));

    TrackPointer pTrack = pTrackModel->getTrack(m_currentEditedCellIndex);
    if (pTrack && pTrack != pOldTrack) {
        emit(loadTrackToPlayer(pTrack, group));
        playStatus->slotSet(1.0);
    } else if (pTrack == pOldTrack && playStatus->get()==0.0) {
        playStatus->slotSet(1.0);
    } else {
        playStatus->slotSet(0.0);
    }
    delete playStatus;
}
