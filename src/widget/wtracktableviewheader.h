#pragma once

#include <QHeaderView>
#include <QMap>
#include <QMenu>

#include "proto/headers.pb.h"

class TrackModel;
class QAction;
class QCheckBox;
class QContextMenuEvent;
class QWidget;

// Thanks to StackOverflow http://stackoverflow.com/questions/1163030/qt-qtableview-and-horizontalheader-restorestate
// answer with this code snippet: http://codepad.org/2gPIMPYU
class HeaderViewState {
  public:
    HeaderViewState() {}

    // Populate the object based on the provided live view.
    explicit HeaderViewState(const QHeaderView& headers);

    // Populate from an existing protobuf, mostly for testing.
    explicit HeaderViewState(const mixxx::library::HeaderViewState& pb)
            : m_view_state(pb) { }

    // Populate the object with the serialized protobuf data provided.
    explicit HeaderViewState(const QString& base64serialized);

    // Returns a serialized protobuf of the current state.
    QString saveState() const;
    // Apply the state to the provided view. The data in the object may be
    // changed if the header format has changed.
    // Don't sort if explicitly disabled, for example when cloning the Tracks header.
    void restoreState(QHeaderView* pHeaders, bool sort = true);

    // returns false if no headers are listed to be shown.
    bool healthy() const {
        if (m_view_state.header_state_size() == 0) {
            return false;
        }
        for (int i = 0; i < m_view_state.header_state_size(); ++i) {
            if (!m_view_state.header_state(i).hidden()) {
                return true;
            }
        }
        return false;
    }

  private:
    mixxx::library::HeaderViewState m_view_state;
};


class WTrackTableViewHeader : public QHeaderView {
    Q_OBJECT
  public:
    explicit WTrackTableViewHeader(Qt::Orientation orientation, QWidget* pParent = nullptr);

    void contextMenuEvent(QContextMenuEvent* event) override;
    void setModel(QAbstractItemModel* model) override;

    void saveHeaderState();
    void restoreHeaderState();
    void loadDefaultHeaderState();
    // Try to load the header state of the 'Tracks' view.
    // Should always be valid because Tracks is selected by default after start.
    void loadCustomHeaderState();
    void storeAsCustomHeaderState();

    // returns true if the header state is stored in the database (false on first time usage)
    bool hasPersistedHeaderState();

  signals:
    void shuffle();

  private slots:
    void showOrHideColumn(int);

  private:
    int hiddenCount();
    void updateMenu();
    TrackModel* getTrackModel();

    QMenu m_menu;
    QMap<int, QCheckBox*> m_columnCheckBoxes;
};
