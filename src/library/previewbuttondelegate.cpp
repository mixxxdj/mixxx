#include <QPainter>
#include <QPushButton>
#include <QTableView>

#include "library/previewbuttondelegate.h"
#include "library/trackmodel.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "track/track.h"
#include "control/controlproxy.h"

PreviewButtonDelegate::PreviewButtonDelegate(QTableView* parent, int column)
        : TableItemDelegate(parent),
          m_pTableView(parent),
          m_isOneCellInEditMode(false),
          m_column(column) {
    m_pPreviewDeckPlay = new ControlProxy(
            PlayerManager::groupForPreviewDeck(0), "play", this);
    m_pPreviewDeckPlay->connectValueChanged(this, &PreviewButtonDelegate::previewDeckPlayChanged);

    m_pCueGotoAndPlay = new ControlProxy(
            PlayerManager::groupForPreviewDeck(0), "cue_gotoandplay", this);

    // This assumes that the parent is wtracktableview
    connect(this, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)),
            parent, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)));

    // The button needs to be parented to receive the parent styles.
    m_pButton = make_parented<LibraryPreviewButton>(m_pTableView);
    m_pButton->setCheckable(true);
    m_pButton->setChecked(false);

    // We need to hide the button that it is not painted by the QObject tree
    m_pButton->hide();

    connect(m_pTableView, SIGNAL(entered(QModelIndex)),
            this, SLOT(cellEntered(QModelIndex)));
}

PreviewButtonDelegate::~PreviewButtonDelegate() {
}

QWidget* PreviewButtonDelegate::createEditor(QWidget* parent,
                                             const QStyleOptionViewItem& option,
                                             const QModelIndex& index) const {
    Q_UNUSED(option);
    QPushButton* btn = new LibraryPreviewButton(parent);
    btn->setCheckable(true);
    // Prevent being focused by Tab key or emulated Tab sent by library controls
    // Avoids a Tab loop caused by setLibraryFocus() when the mouse pointer
    // is above the button (Prev.btn > Table > Prev.btn > ...) and also
    // keeps the table focus border when the hovered button's row is selected
    btn->setFocusPolicy(Qt::NoFocus);
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

void PreviewButtonDelegate::setEditorData(QWidget* editor,
                                          const QModelIndex& index) const {
    Q_UNUSED(editor);
    Q_UNUSED(index);
}

void PreviewButtonDelegate::setModelData(QWidget* editor,
                                         QAbstractItemModel* model,
                                         const QModelIndex& index) const {
    Q_UNUSED(editor);
    Q_UNUSED(model);
    Q_UNUSED(index);
}

void PreviewButtonDelegate::paintItem(QPainter* painter,
                                      const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const {
    // Let the editor paint in this case
    if (index == m_currentEditedCellIndex) {
        return;
    }

    if (!m_pButton) {
        return;
    }

    // We only need m_pButton to have the right width/height, since we are
    // calling its render method directly. Every resize/translate of a widget
    // causes Qt to flush the backing store, so we need to avoid this whenever
    // possible.
    if (option.rect.size() != m_pButton->size()) {
        m_pButton->setFixedSize(option.rect.size());
    }

    // Check-state is whether the track is loaded (index.data()) and whether
    // it's playing.
    m_pButton->setChecked(index.data().toBool() && m_pPreviewDeckPlay->toBool());

    // Render button at the desired position.
    painter->translate(option.rect.topLeft());

    // Avoid QWidget::render and call the equivalent of QPushButton::paintEvent
    // directly.
    m_pButton->paint(painter);
}

void PreviewButtonDelegate::updateEditorGeometry(QWidget* editor,
                                                 const QStyleOptionViewItem& option,
                                                 const QModelIndex& index) const {
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

QSize PreviewButtonDelegate::sizeHint(const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    if (!m_pButton) {
        return QSize();
    }
    return m_pButton->sizeHint();
}

void PreviewButtonDelegate::cellEntered(const QModelIndex& index) {
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
