#include "widget/wtracktableviewheader.h"

#include <QCheckBox>
#include <QContextMenuEvent>
#include <QPainter>
#include <QStyleOptionHeader>
#include <QTextOption>
#include <QWidgetAction>

#include "library/trackmodel.h"
#include "moc_wtracktableviewheader.cpp"
#include "util/math.h"
#include "util/painterscope.h"
#include "util/parented_ptr.h"
#include "widget/wmenucheckbox.h"

#define WTTVH_MINIMUM_SECTION_SIZE 20

HeaderViewState::HeaderViewState(const WTrackTableViewHeader& headers) {
    QAbstractItemModel* model = headers.model();
    for (int vi = 0; vi < headers.count(); ++vi) {
        int li = headers.logicalIndex(vi);
        mixxx::library::HeaderViewState::HeaderState* header_state =
                m_view_state.add_header_state();
        header_state->set_hidden(headers.isSectionHidden(li));
        // Unfortunately sectionSize() is always 0 for hidden columns. Though,
        // QHeaderView keeps track of hidden sizes internally, and we do the same.
        int size = headers.sectionSize(li);
        if (headers.isSectionHidden(li) && size == 0) {
            size = headers.getWidthOfHiddenColumn(li);
            if (size == 0) {
                // Indicates a hidden column we didn't enable yet, hence didn't
                // store its previous width when hiding it.
                // Let's get the default width from the track model.
                // If this also returns 0 for some reason, we'll reset to minimum
                // width when restoring the header.
                auto* pTrackModel = headers.model();
                DEBUG_ASSERT(pTrackModel);
                size = pTrackModel->headerData(
                                          li,
                                          headers.orientation(),
                                          TrackModel::kHeaderWidthRole)
                               .toInt();
            }
        }
        header_state->set_size(size);
        header_state->set_logical_index(li);
        header_state->set_visual_index(vi);
        const QString column_name = model->headerData(
                                                 li, Qt::Horizontal, TrackModel::kHeaderNameRole)
                                            .toString();
        // If there was some sort of error getting the column id,
        // we have to skip this one. (Happens with non-displayed columns)
        if (column_name.isEmpty()) {
            continue;
        }
        header_state->set_column_name(column_name.toStdString());
    }
    m_view_state.set_sort_indicator_shown(headers.isSortIndicatorShown());
    if (m_view_state.sort_indicator_shown()) {
        m_view_state.set_sort_indicator_section(headers.sortIndicatorSection());
        m_view_state.set_sort_order(
                static_cast<int>(headers.sortIndicatorOrder()));
    }
}

HeaderViewState::HeaderViewState(const QString& base64serialized) {
    // First decode the array from Base64, then initialize the protobuf from it.
    QByteArray array = QByteArray::fromBase64(base64serialized.toLatin1());
    if (!m_view_state.ParseFromArray(array.constData(), array.size())) {
        qWarning() << "Could not parse m_view_state from QByteArray of size "
                   << array.size();
        return;
    }
}

QString HeaderViewState::saveState() const {
    // Serialize the proto to a byte array, then encode the array as Base64.
#if GOOGLE_PROTOBUF_VERSION >= 3001000
    int size = static_cast<int>(m_view_state.ByteSizeLong());
#else
    int size = m_view_state.ByteSize();
#endif
    QByteArray array(size, '\0');
    m_view_state.SerializeToArray(array.data(), size);
    return QString(array.toBase64());
}

void HeaderViewState::restoreState(WTrackTableViewHeader* pHeaders) {
    const int max_columns =
            math_min(pHeaders->count(), m_view_state.header_state_size());

    typedef QMap<QString, mixxx::library::HeaderViewState::HeaderState*> state_map;
    state_map map;
    for (int i = 0; i < m_view_state.header_state_size(); ++i) {
        map[QString::fromStdString(m_view_state.header_state(i).column_name())] =
                m_view_state.mutable_header_state(i);
    }

    // First set all sections to be hidden and update logical indexes.
    for (int li = 0; li < pHeaders->count(); ++li) {
        bool hidden = true;
        auto it = map.find(pHeaders->model()->headerData(
                                                    li, Qt::Horizontal, TrackModel::kHeaderNameRole)
                        .toString());
        if (it == map.end()) {
            // This is a column for which the stored state doesn't have a record,
            // so this is likely a new column, added by last update.
            // Enforce visible so it can be discovered.
            qDebug() << "Header view: enforce visibility of new/unknown column"
                     << pHeaders->model()->headerData(
                                                 li, Qt::Horizontal)
                                .toString() // translated name
                     << pHeaders->model()->headerData(
                                                 li, Qt::Horizontal, TrackModel::kHeaderNameRole)
                                .toString(); // internal name
            hidden = false;
        } else {
            it.value()->set_logical_index(li);
        }
        pHeaders->setSectionHidden(li, hidden);
    }

    // Now restore
    for (int vi = 0; vi < max_columns; ++vi) {
        const mixxx::library::HeaderViewState::HeaderState& header =
                m_view_state.header_state(vi);
        const int li = header.logical_index();
        pHeaders->setSectionHidden(li, header.hidden());
        // If the stored size is 0 or less than the minimum column width,
        // we use the latter. This might happen if  WTTVH_MINIMUM_SECTION_SIZE
        // has been increased by us or the header state from database was corrupted.
        // Note: setting the size works even if the column is hidden. Size is stored
        // by QHeaderView internally and is applied once the column is shown.
        int size = math_max(header.size(), WTTVH_MINIMUM_SECTION_SIZE);
        pHeaders->resizeSection(li, size);
        pHeaders->moveSection(pHeaders->visualIndex(li), vi);
    }
    if (m_view_state.sort_indicator_shown()) {
        pHeaders->setSortIndicator(
                m_view_state.sort_indicator_section(),
                static_cast<Qt::SortOrder>(m_view_state.sort_order()));
    }
}

WTrackTableViewHeader::WTrackTableViewHeader(Qt::Orientation orientation,
        QWidget* pParent)
        : QHeaderView(orientation, pParent),
          m_menu(tr("Show or hide columns."), this),
          m_preferredHeight(-1),
          m_hoveredSection(-1),
          m_previousHoveredSection(-1) {
}

void WTrackTableViewHeader::contextMenuEvent(QContextMenuEvent* pEvent) {
    pEvent->accept();
    m_menu.popup(pEvent->globalPos());
}

void WTrackTableViewHeader::setModel(QAbstractItemModel* pModel) {
    TrackModel* pOldTrackModel = getTrackModel();

    if (dynamic_cast<QAbstractItemModel*>(pOldTrackModel) == pModel) {
        // If the models are the same, do nothing but the redundant call.
        QHeaderView::setModel(pModel);
        return;
    }

    // Won't happen in practice since the WTrackTableView new's a new
    // WTrackTableViewHeader each time a new TrackModel is loaded.
    // if (pOldTrackModel) {
    //     saveHeaderState();
    // }

    // First clear all the context menu actions for the old model.
    clearActions();

    // Now set the header view to show the new model
    QHeaderView::setModel(pModel);

    // Now build actions for the new TrackModel
    TrackModel* pTrackModel = dynamic_cast<TrackModel*>(pModel);

    if (!pTrackModel) {
        return;
    }

    // Restore saved header state to get sizes, column positioning, etc. back.
    m_hiddenColumnSizes.clear();
    restoreHeaderState();

    // Here we can override values to prevent restoring corrupt values from database
    setSectionsMovable(true);

    // Setting true in the next line causes Bug #925619 at least with Qt 4.6.1
    setCascadingSectionResizes(false);

    setMinimumSectionSize(WTTVH_MINIMUM_SECTION_SIZE);

    // Create a checkbox for each column.
    // We want to keep the menu open after un/ticking a box because that allows
    // to toggle multiple columns in one go, i.e. without having to open the
    // menu again and again. This does not work with regular QActions so we
    // create QCheckboxes inside QWidgetAction.
    // * toggle a box with mouse click or Space on a selected box (via keyboard,
    //   not just hovered by mouse pointer)
    // * toggle and close by pressing Return on a selected box
    int columns = pModel->columnCount();
    for (int i = 0; i < columns; ++i) {
        if (pTrackModel->isColumnInternal(i)) {
            continue;
        }

        const QString title = pModel->headerData(i, orientation()).toString();

        // Custom QCheckBox with fixed hover behavior
        auto pCheckBox = make_parented<WMenuCheckBox>(title, &m_menu);
        // Keep a map of checkboxes and columns
        m_columnCheckBoxes.insert(i, pCheckBox.get());
        connect(pCheckBox.get(),
                &QCheckBox::toggled,
                this,
                [this, i] {
                    showOrHideColumn(i);
                });
        // If Mixxx starts the first time or the header states have been cleared
        // due to database schema evolution we gonna hide all columns that may
        // contain a potential large number of NULL values.  Here we uncheck
        // the items that are hidden by default (e.g., key column).
        if (!hasPersistedHeaderState() && pTrackModel->isColumnHiddenByDefault(i)) {
            pCheckBox->setChecked(false);
        } else {
            pCheckBox->setChecked(!isSectionHidden(i));
        }

        auto pAction = make_parented<QWidgetAction>(this);
        pAction->setDefaultWidget(pCheckBox.get());
        // Pressing Return triggers the action but that would not toggle the
        // checkbox, we need to do this ourselves while the menu is being closed.
        connect(pAction,
                &QAction::triggered,
                this,
                [pCheckBox{pCheckBox.get()}] {
                    pCheckBox->toggle();
                });
        m_menu.addAction(pAction);

    }

    m_menu.addSeparator();

    // Only show the shuffle action in models that allow sorting.
    if (pTrackModel->hasCapabilities(TrackModel::Capability::Sorting)) {
        auto pShuffleAction = make_parented<QAction>(tr("Shuffle Tracks"), &m_menu);
        connect(pShuffleAction,
                &QAction::triggered,
                this,
                &WTrackTableViewHeader::shuffle,
                /*signal-to-signal*/ Qt::DirectConnection);
        m_menu.addAction(pShuffleAction);
    }

    // Safety check against someone getting stuck with all columns hidden
    // (produces an empty library table). Just re-show them all.
    if (hiddenCount() == columns) {
        for (int i = 0; i < columns; ++i) {
            showSection(i);
        }
    }
}

void WTrackTableViewHeader::saveHeaderState() {
    TrackModel* pTrackModel = getTrackModel();
    if (!pTrackModel) {
        return;
    }
    // Convert the QByteArray to a Base64 string and save it.
    HeaderViewState view_state(*this);
    pTrackModel->setModelSetting("header_state_pb", view_state.saveState());
    //qDebug() << "Saving old header state:" << result << headerState;
}

void WTrackTableViewHeader::restoreHeaderState() {
    TrackModel* pTrackModel = getTrackModel();

    if (!pTrackModel) {
        return;
    }

    const QString headerStateString = pTrackModel->getModelSetting("header_state_pb");
    if (headerStateString.isNull()) {
        loadDefaultHeaderState();
    } else {
        // Load the previous header state (stored as serialized protobuf).
        // Decode it and restore it.
        //qDebug() << "Restoring header state from proto" << headerStateString;
        HeaderViewState view_state(headerStateString);
        if (!view_state.healthy()) {
            loadDefaultHeaderState();
        } else {
            view_state.restoreState(this);
        }
    }
}

void WTrackTableViewHeader::loadDefaultHeaderState() {
    // TODO: isColumnHiddenByDefault logic probably belongs here now.
    QAbstractItemModel* pModel = model();
    for (int i = 0; i < count(); ++i) {
        int header_size = pModel->headerData(
                                        i, orientation(), TrackModel::kHeaderWidthRole)
                                  .toInt();
        if (header_size > 0) {
            resizeSection(i, header_size);
        }
    }
}

bool WTrackTableViewHeader::hasPersistedHeaderState() {
    TrackModel* pTrackModel = getTrackModel();
    if (!pTrackModel) {
        return false;
    }
    const QString headerStateString = pTrackModel->getModelSetting("header_state_pb");
    return !headerStateString.isNull();
}

void WTrackTableViewHeader::clearActions() {
    // The QActions are parented to the menu, so clearing deletes them. Since
    // they are deleted we don't have to disconnect their signals from the
    // mapper.
    m_columnCheckBoxes.clear();
    m_menu.clear();
}

void WTrackTableViewHeader::showOrHideColumn(int column) {
    auto it = m_columnCheckBoxes.constFind(column);
    if (it == m_columnCheckBoxes.constEnd()) {
        qWarning() << "WTrackTableViewHeader got invalid column" << column;
        return;
    }
    QCheckBox* pCheckBox = it.value();
    if (pCheckBox->isChecked()) {
        showSection(column);
        VERIFY_OR_DEBUG_ASSERT(sectionSize(column) >= WTTVH_MINIMUM_SECTION_SIZE) {
            resizeSection(column, WTTVH_MINIMUM_SECTION_SIZE);
        }
        m_hiddenColumnSizes.remove(column);
    } else {
        // If the user hides every column, the table will disappear. This guards
        // against that. Note: hiddenCount reflects number of checked QActions,
        // so size - hiddenCount will be zero the moment they uncheck the last
        // section.
        if (m_columnCheckBoxes.size() - hiddenCount() > 0) {
            m_hiddenColumnSizes.insert(column, sectionSize(column));
            hideSection(column);
        } else {
            // Otherwise, ignore the request and re-check this QAction.
            pCheckBox->setChecked(true);
        }
    }
}

int WTrackTableViewHeader::getWidthOfHiddenColumn(int column) const {
    const auto& it = m_hiddenColumnSizes.find(column);
    if (it != m_hiddenColumnSizes.constEnd()) {
        return it.value();
    }
    return 0;
}

int WTrackTableViewHeader::hiddenCount() {
    int count = 0;
    for (const auto& pCheckBox : std::as_const(m_columnCheckBoxes)) {
        if (!pCheckBox->isChecked()) {
            count += 1;
        }
    }
    return count;
}

TrackModel* WTrackTableViewHeader::getTrackModel() {
    TrackModel* pTrackModel = dynamic_cast<TrackModel*>(model());
    return pTrackModel;
}

void WTrackTableViewHeader::setFont(const QFont& font) {
    // Note:
    // This does not necessarily set the "font" -- QStylesheetStyle seems to be
    // so dominant/persistent that setFont() only adopts font properties which
    // we did NOT set in qss.
    // So, for
    // WTrackTableViewHeader, WTrackTableViewHeader::section {
    //   font-family: "Open Sans Semibold", "Open Sans";
    //   font-weight: 500; /* semi-bold */
    //   font-style: normal;
    //   text-transform: none;
    //   but not font-size; }
    // setFont() does only set font-size.
    QHeaderView::setFont(font);

    // When we set a new font, QHeaderView's QStyle kind of adopts the stylesheet
    // we applied (border and padding are intact), but apparently adds some content
    // margin in QStyle::SE_HeaderLabel which changes the effective text padding.
    // Since we can't re-apply the stylesheet, we need to calculate the original
    // text padding (border + padding) in order to get the new preferred height.
    // This is then used by sizeHint() to return the preferred size.
    // Note: we don't touch the column width.
    setHeightForFont();

    // In order to update the view instantly we need to make some update calls.
    // Note: order of these seems to be crucial. Otherwise we wouldn't get
    // the new size until we switch the track model (create a new header)
    updateGeometry(); // Tells layout to re-query sizeHint()
    adjustSize();     // Resizes to sizeHint()
    update();         // Repaints the widget
}

void WTrackTableViewHeader::setHeightForFont() {
    // Re-calculate the fixed height used by sizeHint().
    // Subtract content height from frame height to get the
    // vertical padding, and add that to the font height to get the new ttotal height.
    QStyleOptionHeader opt;
    initStyleOption(&opt);
    const QRect baseRect(QPoint(0, 0), QHeaderView::sizeHint());
    opt.rect = baseRect;
    const QRect contentRect = style()->subElementRect(QStyle::SE_HeaderLabel, &opt, this);
    int vPadding = baseRect.height() - contentRect.height();
    // This gives the desired result (capital height + ascent + descent).
    // font().pixelSize() / QFontInfo(font()).pixelSize() will apparently return
    // something like font(),capHeight() which is not adequate here.
    QFontMetrics fm(font());
    m_preferredHeight = fm.height() + vPadding;
}

void WTrackTableViewHeader::leaveEvent(QEvent* pEvent) {
    if (m_hoveredSection != -1) {
        m_previousHoveredSection = m_hoveredSection;
        m_hoveredSection = -1;
        updateSection(m_previousHoveredSection);
    }

    QHeaderView::leaveEvent(pEvent);
}

void WTrackTableViewHeader::mouseMoveEvent(QMouseEvent* pEvent) {
    int hovered = logicalIndexAt(pEvent->pos());

    // We need to track the hover state manually, it's not set when initializing
    // the QStyleOption in paintSection()
    if (hovered != m_hoveredSection) {
        // Store previous section before updating
        m_previousHoveredSection = m_hoveredSection;
        m_hoveredSection = hovered;

        if (m_previousHoveredSection != -1) {
            updateSection(m_previousHoveredSection);
        }
        if (m_hoveredSection != -1) {
            updateSection(m_hoveredSection);
        }
    }

    QHeaderView::mouseMoveEvent(pEvent);
}

QSize WTrackTableViewHeader::sizeHint() const {
    if (m_preferredHeight == -1) { // no font set by us, yet
        return QHeaderView::sizeHint();
    }
    return QSize(QHeaderView::sizeHint().width(), m_preferredHeight);
}

void WTrackTableViewHeader::paintSection(
        QPainter* pPainter,
        const QRect& rect,
        int logicalIndex) const {
    // Work around a Qt bug that occurs when we set sort icons in qss (and we need to
    // because setting any QHeaderView::section style property clears the default icons):
    // the style would reserve space for the sort indicator, even in header sections
    // where it's not visible, and thereby truncate the text.
    //
    // Probably this one:
    // https://bugreports.qt.io/browse/QTBUG-27038
    // Previous Mixxx workaround which is not applicable anymore since we are now
    // applying the library font to the header at runtime:
    // https://github.com/mixxxdj/mixxx/pull/13535
    //
    // Fix: draw background & border, then text, then sort indicator.
    // This is a stripped and fixed adaption of QHeaderView::paintSection()
    QStyleOptionHeader opt;
    initStyleOption(&opt);
    opt.rect = rect;
    opt.section = logicalIndex;
    // Consider hover state to apply the respective style from qss
    if (logicalIndex == m_hoveredSection) {
        opt.state |= QStyle::State_MouseOver;
    }

    // Draw background & border only
    opt.text = QString(); // prevent style from drawing the text
    opt.icon = QIcon();   // prevent icon overlap
    style()->drawControl(QStyle::CE_HeaderSection, &opt, pPainter, this);

    const QRect contentRect = style()->subElementRect(QStyle::SE_HeaderLabel, &opt, this);
    // Note: contentRect.height() is now actually equal to m_preferredHeight

    { // Draw text. Use PainterScope, just in case...
        PainterScope painterScope(pPainter);

        // BaseTrackTableModel::headerData(section, orientation, Qt::TextAlignmentRole)
        // replaces the vertical component of the default alignment flags with VCenter.
        const Qt::Alignment alignment = model()->headerData(logicalIndex,
                                                       Qt::Horizontal,
                                                       Qt::TextAlignmentRole)
                                                .value<Qt::Alignment>();
        QTextOption textOption(alignment);
        textOption.setWrapMode(QTextOption::NoWrap);
        const QString title =
                model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toString();
        // QPainter still has the old font (size)
        pPainter->setFont(font());
        pPainter->setPen(opt.palette.color(QPalette::ButtonText));
        pPainter->drawText(contentRect, title, textOption);
    }

    // Draw sort indicator if needed
    if (isSortIndicatorShown() && sortIndicatorSection() == logicalIndex) {
        // Use the style's original indicator rect but make width = height
        const QRect origIndiRect = style()->subElementRect(QStyle::SE_HeaderArrow, &opt, this);
        int indiWH = origIndiRect.height();
        int indiRectLeft = layoutDirection() == Qt::LeftToRight
                ? origIndiRect.right() - indiWH
                : origIndiRect.left();
        opt.rect = QRect(indiRectLeft, origIndiRect.top(), indiWH, indiWH);

        // NOTE: Don't use drawPrimitive(PE_IndicatorHeaderArrow) because of its
        // platform-specific arrow flipping logic in QFusionStyle::drawPrimitive
        // (ascending = up on Linux, down on Windows/macOS) which is not used by
        // QHeaderView's default painting via
        // QStyleSheetStyle::drawControl(CE_Header|CE_HeaderLabel).
        // Instead, we use the fail-safe method of drawing up/down arrows explicitly
        // with drawPrimitive(PE_IndicatorArrowUp|Down) for consistent appearance.
        // This also updates the widget so qss icons are applied immediately.
        style()->drawPrimitive((sortIndicatorOrder() == Qt::AscendingOrder)
                        ? QStyle::PE_IndicatorArrowUp
                        : QStyle::PE_IndicatorArrowDown,
                &opt,
                pPainter,
                this);
    }
}
