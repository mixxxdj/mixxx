#include "library/tabledelegates/previewbuttondelegate.h"

#include <QPainter>
#include <QPixmapCache>
#include <QPushButton>
#include <QStyleOptionButton>

#include "control/controlproxy.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "moc_previewbuttondelegate.cpp"
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

LibraryPreviewButton::LibraryPreviewButton(QWidget* parent)
        : QPushButton(parent) {
    setObjectName("LibraryPreviewButton");
}

void LibraryPreviewButton::paint(QPainter* painter) {
    // This matches the implementation of QPushButton::paintEvent, except it
    // does not create a new QStylePainter, and it is simpler and more
    // direct than QWidget::render(QPainter*, ...).
    QStyleOptionButton option;
    initStyleOption(&option);
    auto* pStyle = style();
    if (pStyle) {
        pStyle->drawControl(QStyle::CE_PushButton, &option, painter, this);
    }
}

PreviewButtonDelegate::PreviewButtonDelegate(
        WLibraryTableView* parent,
        int column)
        : TableItemDelegate(parent),
          m_column(column),
          m_pPreviewDeckPlay(make_parented<ControlProxy>(
                  kPreviewDeckGroup, QStringLiteral("play"), this)),
          m_pCueGotoAndPlay(kPreviewDeckGroup, QStringLiteral("cue_gotoandplay")),
          m_pButton(make_parented<LibraryPreviewButton>(parent)) {
    DEBUG_ASSERT(m_column >= 0);

    m_pPreviewDeckPlay->connectValueChanged(
            this,
            &PreviewButtonDelegate::previewDeckPlayChanged);

    // This assumes that the parent is/inherits WLibraryTableView
    connect(this,
            &PreviewButtonDelegate::loadTrackToPlayer,
            parent,
            &WLibraryTableView::loadTrackToPlayer);

    // The button needs to be parented to receive the parent styles.
    m_pButton->setCheckable(true);
    m_pButton->setChecked(false);

    // We need to hide the button that it is not painted by the QObject tree
    m_pButton->hide();

    connect(m_pTableView,
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

    // Generate a unique cache key for this item
    bool isTrackLoadedAndPlaying = isTrackLoadedInPreviewDeckAndPlaying(index);
    QString cacheKey = QString("PreviewButtonDelegate_%1_%2_%3")
                               .arg(option.rect.width())
                               .arg(option.rect.height())
                               .arg(isTrackLoadedAndPlaying ? "playing" : "stopped");

    QPixmap pixmap;
    if (!QPixmapCache::find(cacheKey, &pixmap)) {
        // Pixmap not in cache, create a new one
        pixmap = QPixmap(option.rect.size());
        pixmap.fill(Qt::transparent); // Fill uninitialized pixmap's background transparent
        QPainter pixmapPainter(&pixmap);

        QStyleOptionButton opt;

        // Set the buttons origin to top-left and adjust the size to fit into the pixmap.
        opt.rect = QRect(QPoint(0, 0), option.rect.size());

        m_pButton->setFixedSize(option.rect.size());

        // Update check state
        opt.state = isTrackLoadedAndPlaying ? QStyle::State_On : QStyle::State_Off;

        // Use the QTableView's style to draw onto the pixmap.
        QStyle* style = m_pButton->style();
        if (style != nullptr) {
            style->drawControl(QStyle::CE_PushButton, &opt, &pixmapPainter, m_pButton);
        }

        pixmapPainter.end(); // Finish painting to the pixmap.

        // Cache the newly created pixmap
        QPixmapCache::insert(cacheKey, pixmap);
    }

    // Draw the cached pixmap onto the painter
    painter->drawPixmap(option.rect.topLeft(), pixmap);
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
    VERIFY_OR_DEBUG_ASSERT(index.isValid()) {
        return;
    }
    // Ignore signal if the edited cell index didn't change.
    // Receiving this signal for the same cell again could happen
    // if no other cell has been entered between those events.
    // https://github.com/mixxxdj/mixxx/issues/10418
    if (index == m_currentEditedCellIndex) {
        return;
    }
    // Close the editor when leaving the currently edited cell
    if (m_currentEditedCellIndex.isValid()) {
        m_pTableView->closePersistentEditor(m_currentEditedCellIndex);
        m_currentEditedCellIndex = QModelIndex();
    }
    // Only open a new editor for preview column cells, but not any
    // other cells
    if (index.column() != m_column) {
        return;
    }
    m_pTableView->openPersistentEditor(index);
    m_currentEditedCellIndex = index;
}

void PreviewButtonDelegate::buttonClicked() {
    VERIFY_OR_DEBUG_ASSERT(m_currentEditedCellIndex.isValid()) {
        return;
    }
    TrackModel* const pTrackModel = trackModel(m_pTableView);
    VERIFY_OR_DEBUG_ASSERT(pTrackModel) {
        return;
    }

    TrackPointer pOldTrack = PlayerInfo::instance().getTrackInfo(kPreviewDeckGroup);

    bool startedPlaying = false;
    TrackPointer pTrack = pTrackModel->getTrack(m_currentEditedCellIndex);
    if (pTrack && pTrack != pOldTrack) {
        // Load to preview deck and start playing
        emit loadTrackToPlayer(pTrack, kPreviewDeckGroup, true);
        startedPlaying = true;
    } else if (pTrack == pOldTrack && !isPreviewDeckPlaying()) {
        // Since the Preview deck might be hidden, starting at the main cue
        // is a predictable behavior.
        m_pCueGotoAndPlay.set(1.0);
        startedPlaying = true;
    } else {
        m_pPreviewDeckPlay->set(0.0);
    }
    // If we start previewing also select the track (the table view didn't receive the click)
    if (startedPlaying) {
        m_pTableView->selectRow(m_currentEditedCellIndex.row());
    }
}

void PreviewButtonDelegate::previewDeckPlayChanged(double v) {
    m_pTableView->update();
    if (!m_currentEditedCellIndex.isValid()) {
        return;
    }
    TrackModel* const pTrackModel = trackModel(m_pTableView);
    VERIFY_OR_DEBUG_ASSERT(pTrackModel) {
        return;
    }
    TrackPointer pPreviewTrack = PlayerInfo::instance().getTrackInfo(kPreviewDeckGroup);
    TrackPointer pTrack = pTrackModel->getTrack(m_currentEditedCellIndex);
    if (pTrack && pTrack == pPreviewTrack) {
        emit buttonSetChecked(v > 0.0);
    }
}
