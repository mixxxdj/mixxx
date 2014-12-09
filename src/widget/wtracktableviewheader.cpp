// wtracktableviewheader.cpp
// Created 1/2/2010 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "widget/wtracktableviewheader.h"
#include "library/trackmodel.h"
#include "proto/headers.pb.h"

#define WTTVH_MINIMUM_SECTION_SIZE 20

QHeaderViewState::QHeaderViewState(const QHeaderView& headers)
{
    for (int vi = 0; vi < headers.count(); ++vi) {
        int li = headers.logicalIndex(vi);
        HeaderState header;
        header.hidden = headers.isSectionHidden(li);
        header.size = headers.sectionSize(li);
        header.logical_index = li;
        header.visual_index = vi;
        int column_id = headers.model()->headerData(
                li, Qt::Horizontal, TrackModel::kHeaderIDRole).toInt();
        // If there was some sort of error getting the column id,
        // we have to skip this one.
        if (column_id == -1) {
            continue;
        }
        header.column_id = column_id;
        m_headers.append(header);
    }
    m_sort_indicator_shown = headers.isSortIndicatorShown();
    if (m_sort_indicator_shown) {
        m_sort_indicator_section = headers.sortIndicatorSection();
        m_sort_order = headers.sortIndicatorOrder();
    }
}

QHeaderViewState::QHeaderViewState(const QString& serialized) {
    mixxx::library::HeaderViewState headerViewState_pb;
    headerViewState_pb.ParseFromString(serialized.toStdString());

    for (int i = 0; i < headerViewState_pb.header_state_size(); ++i) {
        const mixxx::library::HeaderViewState::HeaderState& header_state_pb =
                headerViewState_pb.header_state(i);
        HeaderState header;
        header.hidden = header_state_pb.hidden();
        header.size = header_state_pb.size();
        header.logical_index = header_state_pb.logical_index();
        header.visual_index = header_state_pb.visual_index();
        header.column_id = header_state_pb.column_id();
        m_headers.append(header);
    }

    m_sort_indicator_shown = headerViewState_pb.sort_indicator_shown();
    m_sort_indicator_section = headerViewState_pb.sort_indicator_section();
    m_sort_order = static_cast<Qt::SortOrder>(headerViewState_pb.sort_order());
}

void QHeaderViewState::restoreState(QHeaderView* headers) const {
    const int max_columns = std::min(headers->count(),
                                     static_cast<int>(m_headers.size()));

    // Need to make a copy for constness
    QList<HeaderState> header_state(m_headers);
    QMap<int, HeaderState*> map;
    for (int i = 0; i < header_state.size(); ++i) {
        map[header_state[i].column_id] = &header_state[i];
    }

    // First set all sections to be hidden and update logical
    // indexes
    for (int li = 0; li < headers->count(); ++li) {
        headers->setSectionHidden(li, true);
        QMap<int, HeaderState *>::iterator it = map.find(
                headers->model()->headerData(
                        li, Qt::Horizontal, TrackModel::kHeaderIDRole).toInt());
        if (it != map.end()) {
            it.value()->logical_index = li;
        }
    }

    // Now restore
    for (int vi = 0; vi < max_columns; ++vi) {
        HeaderState const & header = header_state[vi];
        const int li = header.logical_index;
        //SSCI_ASSERT_BUG(vi == header.visual_index);
        headers->setSectionHidden(li, header.hidden);
        headers->resizeSection(li, header.size);
        headers->moveSection(headers->visualIndex(li), vi);
    }
    if (m_sort_indicator_shown) {
        headers->setSortIndicator(m_sort_indicator_section, m_sort_order);
    }
}

QString QHeaderViewState::saveState() const {
    // Generate a proto from the internal state and return a string-serialized
    // version.
    mixxx::library::HeaderViewState headerViewState_pb;

    for (int i = 0; i < m_headers.length(); ++i) {
        mixxx::library::HeaderViewState::HeaderState* header_state_pb =
                headerViewState_pb.add_header_state();
        const HeaderState& header = m_headers.at(i);
        header_state_pb->set_hidden(header.hidden);
        header_state_pb->set_size(header.size);
        header_state_pb->set_logical_index(header.logical_index);
        header_state_pb->set_visual_index(header.visual_index);
        header_state_pb->set_column_id(header.column_id);
    }

    headerViewState_pb.set_sort_indicator_shown(m_sort_indicator_shown);
    headerViewState_pb.set_sort_indicator_section(m_sort_indicator_section);
    headerViewState_pb.set_sort_order(static_cast<int>(m_sort_order));

    return QString::fromStdString(headerViewState_pb.SerializeAsString());
}

WTrackTableViewHeader::WTrackTableViewHeader(Qt::Orientation orientation,
                                             QWidget* parent)
        : QHeaderView(orientation, parent),
          m_menu(tr("Show or hide columns."), this),
          m_signalMapper(this) {
    connect(&m_signalMapper, SIGNAL(mapped(int)),
            this, SLOT(showOrHideColumn(int)));
}

WTrackTableViewHeader::~WTrackTableViewHeader() {
}

void WTrackTableViewHeader::contextMenuEvent(QContextMenuEvent* event) {
    m_menu.popup(event->globalPos());
}

void WTrackTableViewHeader::setModel(QAbstractItemModel* model) {
    TrackModel* oldTrackModel = getTrackModel();

    if (dynamic_cast<QAbstractItemModel*>(oldTrackModel) == model) {
        // If the models are the same, do nothing but the redundant call.
        QHeaderView::setModel(model);
        return;
    }

    // Won't happen in practice since the WTrackTableView new's a new
    // WTrackTableViewHeader each time a new TrackModel is loaded.
    // if (oldTrackModel) {
    //     saveHeaderState();
    // }

    // First clear all the context menu actions for the old model.
    clearActions();

    // Now set the header view to show the new model
    QHeaderView::setModel(model);

    // Now build actions for the new TrackModel
    TrackModel* trackModel = dynamic_cast<TrackModel*>(model);

    if (!trackModel) {
        return;
    }

    // Restore saved header state to get sizes, column positioning, etc. back.
    restoreHeaderState();

    // Here we can override values to prevent restoring corrupt values from database
    setMovable(true);

    // Setting true in the next line causes Bug #925619 at least with Qt 4.6.1
    setCascadingSectionResizes(false);

    setMinimumSectionSize(WTTVH_MINIMUM_SECTION_SIZE);

    int columns = model->columnCount();
    for (int i = 0; i < columns; ++i) {
        if (trackModel->isColumnInternal(i)) {
            continue;
        }

        QString title = model->headerData(i, orientation()).toString();
        QAction* action = new QAction(title, &m_menu);
        action->setCheckable(true);

        /* If Mixxx starts the first time or the header states have been cleared
         * due to database schema evolution we gonna hide all columns that may
         * contain a potential large number of NULL values.  Here we uncheck
         * item in the context menu that are hidden by defualt (e.g., key
         * column)
         */
        if (!hasPersistedHeaderState() &&
            trackModel->isColumnHiddenByDefault(i)) {
            action->setChecked(false);
        } else {
            action->setChecked(!isSectionHidden(i));
        }

        // Map this action's signals via our QSignalMapper
        m_signalMapper.setMapping(action, i);
        m_columnActions.insert(i, action);
        connect(action, SIGNAL(triggered()),
                &m_signalMapper, SLOT(map()));
        m_menu.addAction(action);

        // force the section size to be a least WTTVH_MINIMUM_SECTION_SIZE
        if (sectionSize(i) <  WTTVH_MINIMUM_SECTION_SIZE) {
            // This might happen if  WTTVH_MINIMUM_SECTION_SIZ has changed or
            // the header state from database was corrupt
            resizeSection(i,WTTVH_MINIMUM_SECTION_SIZE);
        }
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
    TrackModel* track_model = getTrackModel();
    if (!track_model) {
        return;
    }
    // Convert the QByteArray to a Base64 string and save it.
    QHeaderViewState view_state(*this);
    track_model->setModelSetting("header_state_pb", view_state.saveState());
    //qDebug() << "Saving old header state:" << result << headerState;
}

void WTrackTableViewHeader::restoreHeaderState() {
    TrackModel* track_model = getTrackModel();

    if (!track_model) {
        return;
    }

    QString headerStateString = track_model->getModelSetting("header_state_pb");
    if (!headerStateString.isNull()) {
        // Load the previous header state (stored as serialized protobuf).
        // Decode it and restore it.
        //qDebug() << "Restoring header state from proto" << headerStateString;
        QHeaderViewState view_state(headerStateString);
        view_state.restoreState(this);
        return;
    }

    // Try loading from the old QT-specific format next.
    headerStateString = track_model->getModelSetting("header_state");
    if (!headerStateString.isNull()) {
        // Load the previous header state (stored as a Base 64 string). Decode
        // it and restore it.
        //qDebug() << "Restoring header state from blob" << headerStateString;
        QByteArray headerState = headerStateString.toAscii();
        headerState = QByteArray::fromBase64(headerState);
        restoreState(headerState);
        return;
    }
    loadDefaultHeaderState();
}

void WTrackTableViewHeader::loadDefaultHeaderState() {
    for (int i = 0; i < count(); ++i) {
        int header_size = model()->headerData(
                i, orientation(), TrackModel::kHeaderWidthRole).toInt();
        if (header_size > 0) {
            resizeSection(i, header_size);
        }
    }
}

bool WTrackTableViewHeader::hasPersistedHeaderState() {
    TrackModel* track_model = getTrackModel();
    if (!track_model) {
        return false;
    }
    QString headerStateString = track_model->getModelSetting("header_state_pb");
    if (!headerStateString.isNull()) {
        return true;
    }

    // If the old blob is still there, use it.
    headerStateString = track_model->getModelSetting("header_state");
    if (!headerStateString.isNull()) {
        return true;
    }
    return false;
}

void WTrackTableViewHeader::clearActions() {
    // The QActions are parented to the menu, so clearing deletes them. Since
    // they are deleted we don't have to disconnect their signals from the
    // mapper.
    m_columnActions.clear();
    m_menu.clear();
}

void WTrackTableViewHeader::showOrHideColumn(int column) {
    if (!m_columnActions.contains(column)) {
        qDebug() << "WTrackTableViewHeader got invalid column" << column;
        return;
    }

    QAction* action = m_columnActions[column];
    if (action->isChecked()) {
        showSection(column);
    } else {
        // If the user hides every column then the table will disappear. This
        // guards against that. NB: hiddenCount reflects checked QAction's so
        // size-hiddenCount will be zero the moment they uncheck the last
        // section.
        if (m_columnActions.size() - hiddenCount() > 0) {
            hideSection(column);
        } else {
            // Otherwise, ignore the request and re-check this QAction.
            action->setChecked(true);
        }
    }
}

int WTrackTableViewHeader::hiddenCount() {
    int count = 0;
    for (QMap<int, QAction*>::iterator it = m_columnActions.begin();
         it != m_columnActions.end(); ++it) {
        QAction* pAction = *it;
        if (!pAction->isChecked())
            count += 1;
    }
    return count;
}

TrackModel* WTrackTableViewHeader::getTrackModel() {
    TrackModel* trackModel = dynamic_cast<TrackModel*>(model());
    return trackModel;
}
