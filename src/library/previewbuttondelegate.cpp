#include "library/previewbuttondelegate.h"

#include <QPainter>
#include <QPushButton>
#include <QTableView>

#include "control/controlproxy.h"
#include "library/trackmodel.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "moc_previewbuttondelegate.cpp"
#include "track/track.h"
#include "widget/wlibrarytableview.h"

namespace {

constexpr int kPreviewDeckIndex = 0;

const QString kPreviewDeckGroup = PlayerManager::groupForPreviewDeck(kPreviewDeckIndex);

inline TrackModel* trackModel(QTableView* pTableView) {
    VERIFY_OR_DEBUG_ASSERT(pTableView) {
        return nullptr;
    }
    return dynamic_cast<TrackModel*>(pTableView->model());
}

} // namespace

PreviewButtonDelegate::PreviewButtonDelegate(
        WLibraryTableView* parent,
        int column)
        : TableItemDelegate(parent),
          m_column(column),
          m_pPreviewDeckPlay(make_parented<ControlProxy>(
                  kPreviewDeckGroup, QStringLiteral("play"), this)),
          m_pCueGotoAndPlay(make_parented<ControlProxy>(
                  kPreviewDeckGroup, QStringLiteral("cue_gotoandplay"), this)),
          m_pButton(make_parented<LibraryPreviewButton>(parent)) {
    DEBUG_ASSERT(m_column >= 0);

    m_pPreviewDeckPlay->connectValueChanged(
            this,
            &PreviewButtonDelegate::previewDeckPlayChanged);

    // This assumes that the parent is wtracktableview
    connect(this,
            &PreviewButtonDelegate::loadTrackToPlayer,
            parent,
            &WLibraryTableView::loadTrackToPlayer);

    // The button needs to be parented to receive the parent styles.
    m_pButton->setCheckable(true);
    m_pButton->setChecked(false);

    // We need to hide the button that it is not painted by the QObject tree
    m_pButton->hide();

    connect(parentTableView(),
            &QTableView::entered,
            this,
            &PreviewButtonDelegate::cellEntered);
}

PreviewButtonDelegate::~PreviewButtonDelegate() = default;

bool PreviewButtonDelegate::isPreviewDeckPlaying() const {
    DEBUG_ASSERT(m_pPreviewDeckPlay);
    return m_pPreviewDeckPlay->toBool();
}

bool PreviewButtonDelegate::isTrackLoadedInPreviewDeck(
        const QModelIndex& index) const {
    DEBUG_ASSERT(index.isValid());
    DEBUG_ASSERT(index.column() == m_column);
    const QVariant previewData = index.data();
    DEBUG_ASSERT(!previewData.isNull());
    DEBUG_ASSERT(previewData.canConvert<bool>());
    return previewData.toBool();
}

QWidget* PreviewButtonDelegate::createEditor(QWidget* parent,
                                             const QStyleOptionViewItem& option,
                                             const QModelIndex& index) const {
    Q_UNUSED(option);
    QPushButton* btn = new LibraryPreviewButton(parent);
    // Prevent being focused by Tab key or emulated Tab sent by library controls
    // Avoids a Tab loop caused by setLibraryFocus() when the mouse pointer
    // is above the button (Prev.btn > Table > Prev.btn > ...) and also
    // keeps the table focus border when the hovered button's row is selected
    btn->setFocusPolicy(Qt::NoFocus);
    btn->setCheckable(true);
    btn->setChecked(isTrackLoadedInPreviewDeckAndPlaying(index));
    connect(btn,
            &QPushButton::clicked,
            this,
            &PreviewButtonDelegate::buttonClicked);
    connect(this,
            &PreviewButtonDelegate::buttonSetChecked,
            btn,
            &QPushButton::setChecked);
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
    paintItemBackground(painter, option, index);

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

    // Update check state
    m_pButton->setChecked(isTrackLoadedInPreviewDeckAndPlaying(index));

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
    DEBUG_ASSERT(index.isValid());
    VERIFY_OR_DEBUG_ASSERT(parentTableView()) {
        return;
    }
    // this slot is called if the mouse pointer enters ANY cell on
    // the QTableView but the code should only be executed on a button
    if (index.column() == m_column) {
        DEBUG_ASSERT(index != m_currentEditedCellIndex);
        if (m_currentEditedCellIndex.isValid()) {
            // Close the editor in the other cell
            parentTableView()->closePersistentEditor(m_currentEditedCellIndex);
        }
        parentTableView()->openPersistentEditor(index);
        m_currentEditedCellIndex = index;
    } else if (m_currentEditedCellIndex.isValid()) {
        // Close editor if the mouse leaves the button
        parentTableView()->closePersistentEditor(m_currentEditedCellIndex);
        m_currentEditedCellIndex = QModelIndex();
    }
}

void PreviewButtonDelegate::buttonClicked() {
    VERIFY_OR_DEBUG_ASSERT(m_currentEditedCellIndex.isValid()) {
        return;
    }
    TrackModel* const pTrackModel = trackModel(parentTableView());
    VERIFY_OR_DEBUG_ASSERT(pTrackModel) {
        return;
    }

    TrackPointer pOldTrack = PlayerInfo::instance().getTrackInfo(kPreviewDeckGroup);

    TrackPointer pTrack = pTrackModel->getTrack(m_currentEditedCellIndex);
    if (pTrack && pTrack != pOldTrack) {
        emit loadTrackToPlayer(pTrack, kPreviewDeckGroup, true);
    } else if (pTrack == pOldTrack && !isPreviewDeckPlaying()) {
        // Since the Preview deck might be hidden
        // Starting at cue is a predictable behavior
        m_pCueGotoAndPlay->set(1.0);
    } else {
        m_pPreviewDeckPlay->set(0.0);
    }
}

void PreviewButtonDelegate::previewDeckPlayChanged(double v) {
    parentTableView()->update();
    if (!m_currentEditedCellIndex.isValid()) {
        return;
    }
    TrackModel* const pTrackModel = trackModel(parentTableView());
    VERIFY_OR_DEBUG_ASSERT(pTrackModel) {
        return;
    }
    TrackPointer pPreviewTrack = PlayerInfo::instance().getTrackInfo(kPreviewDeckGroup);
    TrackPointer pTrack = pTrackModel->getTrack(m_currentEditedCellIndex);
    if (pTrack && pTrack == pPreviewTrack) {
        emit buttonSetChecked(v > 0.0);
    }
}
