#include "library/tabledelegates/colordelegate.h"

#include <QApplication>
#include <QMenu>
#include <QPainter>
#include <QResizeEvent>
#include <QStyle>
#include <QTableView>

#include "library/trackmodel.h"
#include "moc_colordelegate.cpp"
#include "preferences/colorpalettesettings.h"
#include "track/track.h"
#include "util/color/rgbcolor.h"
#include "widget/wcolorpickeraction.h"
#include "widget/wlibrarytableview.h"

namespace {
constexpr const char* kUseRowBackgroundForColorCellProperty =
        "mixxxColorDelegateUseRowBackgroundForColorCell";
} // namespace

ColorDelegate::ColorDelegate(QTableView* pTableView, int column)
        : TableItemDelegate(pTableView),
          m_column(column),
          m_pColorPickerMenu(nullptr),
          m_pColorPickerAction(nullptr) {
    DEBUG_ASSERT(m_column >= 0);

    // Show the color picker menu when a color cell is left-clicked.
    // QAbstractItemView::clicked() is emitted only if press and release
    // happened on the same cell with the left mouse button, so right-click
    // remains reserved for the track menu.
    connect(m_pTableView,
            &QTableView::clicked,
            this,
            &ColorDelegate::cellClicked);
}

void ColorDelegate::paintItem(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    const auto color = mixxx::RgbColor::fromQVariant(index.data());
    const bool useRowBackgroundForColorCell =
            m_pTableView->property(kUseRowBackgroundForColorCellProperty).toBool();

    if (color && !useRowBackgroundForColorCell) {
        painter->fillRect(option.rect, mixxx::RgbColor::toQColor(color));
    } else {
        // Filter out track color that is hidden
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
        } else if (useRowBackgroundForColorCell) {
            paintItemBackground(painter, option, index);
        }
    }

    // Draw a border if the color cell has focus
    if (option.state & QStyle::State_HasFocus) {
        drawBorder(painter, m_focusBorderColor, option.rect);
    }
}

void ColorDelegate::cellClicked(const QModelIndex& index) {
    if (!index.isValid() || index.column() != m_column) {
        return;
    }
    auto* pTrackModel = dynamic_cast<TrackModel*>(m_pTableView->model());
    VERIFY_OR_DEBUG_ASSERT(pTrackModel) {
        return;
    }
    TrackPointer pTrack = pTrackModel->getTrack(index);
    if (!pTrack) {
        return;
    }
    showColorPickerMenu(index, pTrack->getColor());
}

void ColorDelegate::showColorPickerMenu(
        const QModelIndex& index,
        const mixxx::RgbColor::optional_t& color) {
    auto* pLibraryTableView = qobject_cast<WLibraryTableView*>(m_pTableView);
    VERIFY_OR_DEBUG_ASSERT(pLibraryTableView) {
        return;
    }
    const ColorPaletteSettings colorPaletteSettings(pLibraryTableView->getConfig());

    if (!m_pColorPickerMenu) {
        m_pColorPickerMenu = make_parented<QMenu>(m_pTableView);
        m_pColorPickerMenu->setAttribute(Qt::WA_StyledBackground);
        m_pColorPickerMenu->setObjectName("TrackColorPopup");
        m_pColorPickerAction = make_parented<WColorPickerAction>(
                WColorPicker::Option::AllowNoColor,
                colorPaletteSettings.getTrackColorPalette(),
                m_pColorPickerMenu);
        m_pColorPickerMenu->addAction(m_pColorPickerAction);
        connect(m_pColorPickerAction,
                &WColorPickerAction::colorPicked,
                this,
                &ColorDelegate::slotColorPicked);
    }

    m_pColorPickerAction->setColorPalette(colorPaletteSettings.getTrackColorPalette());
    // Tell Qt that the menu size needs to be recalculated, since the palette
    // (and thus the color picker size) may have changed. See the comment on
    // WColorPickerAction::setColorPalette().
    QResizeEvent resizeEvent(QSize(), m_pColorPickerMenu->size());
    QApplication::sendEvent(m_pColorPickerMenu, &resizeEvent);
    m_pColorPickerAction->setSelectedColor(color);

    m_pickedIndex = index;
    // Use the bottom left corner of the cell as starting point for the popup.
    // QMenu::popup() shifts the menu to keep it fully on the screen.
    const QPoint point(m_pTableView->viewport()->mapToGlobal(
            m_pTableView->visualRect(index).bottomLeft()));
    m_pColorPickerMenu->popup(point);
    // m_pColorPickerMenu->popup(m_pTableView->viewport()->mapToGlobal(
    //         m_pTableView->visualRect(index).bottomLeft()));
}

void ColorDelegate::slotColorPicked(mixxx::RgbColor::optional_t color) {
    VERIFY_OR_DEBUG_ASSERT(m_pickedIndex.isValid()) {
        return;
    }
    auto* pTrackModel = dynamic_cast<TrackModel*>(m_pTableView->model());
    VERIFY_OR_DEBUG_ASSERT(pTrackModel) {
        return;
    }
    TrackPointer pTrack = pTrackModel->getTrack(m_pickedIndex);
    if (pTrack) {
        pTrack->setColor(color);
    }
    m_pickedIndex = QModelIndex();
    m_pColorPickerMenu->hide();
}
