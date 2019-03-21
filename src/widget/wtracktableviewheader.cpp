// wtracktableviewheader.cpp
// Created 1/2/2010 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "widget/wtracktableviewheader.h"
#include "library/trackmodel.h"
#include "util/math.h"

#define WTTVH_MINIMUM_SECTION_SIZE 20

HeaderViewState::HeaderViewState(const QHeaderView& headers)
{
    QAbstractItemModel* model = headers.model();
    for (int vi = 0; vi < headers.count(); ++vi) {
        int li = headers.logicalIndex(vi);
        mixxx::library::HeaderViewState::HeaderState* header_state =
                m_view_state.add_header_state();
        header_state->set_hidden(headers.isSectionHidden(li));
        header_state->set_size(headers.sectionSize(li));
        header_state->set_logical_index(li);
        header_state->set_visual_index(vi);
        QString column_name = model->headerData(
                li, Qt::Horizontal, TrackModel::kHeaderNameRole).toString();
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
    QByteArray array;
    array.append(base64serialized);
    // First decode the array from Base64, then initialize the protobuf from it.
    array = QByteArray::fromBase64(array);
    if (!m_view_state.ParseFromArray(array.constData(), array.size())) {
        qDebug() << "ERROR: Could not parse m_view_state from QByteArray of size "
                 << array.size();
        return;
    }
}

QString HeaderViewState::saveState() const {
    // Serialize the proto to a byte array, then encode the array as Base64.
    int size = m_view_state.ByteSize();
    QByteArray array(size, '\0');
    m_view_state.SerializeToArray(array.data(), size);
    return QString(array.toBase64());
}

void HeaderViewState::restoreState(QHeaderView* headers) {
    const int max_columns =
            math_min(headers->count(), m_view_state.header_state_size());

    typedef QMap<QString, mixxx::library::HeaderViewState::HeaderState*> state_map;
    state_map map;
    for (int i = 0; i < m_view_state.header_state_size(); ++i) {
        map[QString::fromStdString(m_view_state.header_state(i).column_name())] =
                m_view_state.mutable_header_state(i);
    }

    // First set all sections to be hidden and update logical indexes.
    for (int li = 0; li < headers->count(); ++li) {
        headers->setSectionHidden(li, true);
        auto it = map.find(headers->model()->headerData(
            li, Qt::Horizontal, TrackModel::kHeaderNameRole).toString());
        if (it != map.end()) {
            it.value()->set_logical_index(li);
        }
    }

    // Now restore
    for (int vi = 0; vi < max_columns; ++vi) {
        const mixxx::library::HeaderViewState::HeaderState& header =
                m_view_state.header_state(vi);
        const int li = header.logical_index();
        headers->setSectionHidden(li, header.hidden());
        headers->resizeSection(li, header.size());
        headers->moveSection(headers->visualIndex(li), vi);
    }
    if (m_view_state.sort_indicator_shown()) {
        headers->setSortIndicator(
                m_view_state.sort_indicator_section(),
                static_cast<Qt::SortOrder>(m_view_state.sort_order()));
    }
}

WTrackTableViewHeader::WTrackTableViewHeader(Qt::Orientation orientation,
                                             QWidget* parent)
        : QHeaderView(orientation, parent),
          m_menu(tr("Show or hide columns."), this),
          m_signalMapper(this) {
    connect(&m_signalMapper, SIGNAL(mapped(int)),
            this, SLOT(showOrHideColumn(int)));
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
    setSectionsMovable(true);

    // Setting true in the next line causes Bug #925619 at least with Qt 4.6.1
    setCascadingSectionResizes(false);

    setMinimumSectionSize(WTTVH_MINIMUM_SECTION_SIZE);

    int columns = model->columnCount();
    for (int i = 0; i < columns; ++i) {
        if (trackModel->isColumnInternal(i)) {
            continue;
        }

        QString title = model->headerData(i, orientation()).toString();
        auto action = new QAction(title, &m_menu);
        action->setCheckable(true);

        /* If Mixxx starts the first time or the header states have been cleared
         * due to database schema evolution we gonna hide all columns that may
         * contain a potential large number of NULL values.  Here we uncheck
         * item in the context menu that are hidden by default (e.g., key
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
    HeaderViewState view_state(*this);
    track_model->setModelSetting("header_state_pb", view_state.saveState());
    //qDebug() << "Saving old header state:" << result << headerState;
}

void WTrackTableViewHeader::restoreHeaderState() {
    TrackModel* track_model = getTrackModel();

    if (!track_model) {
        return;
    }

    QString headerStateString = track_model->getModelSetting("header_state_pb");
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
    QAbstractItemModel* m = model();
    for (int i = 0; i < count(); ++i) {
        int header_size = m->headerData(
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
    return !headerStateString.isNull();
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
    for (const auto& pAction : m_columnActions) {
        if (!pAction->isChecked()) {
            count += 1;
        }
    }
    return count;
}

TrackModel* WTrackTableViewHeader::getTrackModel() {
    TrackModel* trackModel = dynamic_cast<TrackModel*>(model());
    return trackModel;
}
