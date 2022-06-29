#pragma once

#include <QCache>
#include <QFont>
#include <QItemSelectionModel>
#include <QString>
#include <QTableView>

#include "library/libraryview.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"

class TrackModel;

class WLibraryTableView : public QTableView, public virtual LibraryView {
    Q_OBJECT

    struct ModelState {
        int horizontalScrollPosition;
        int verticalScrollPosition;
        QModelIndexList selectedRows;
        QModelIndex currentIndex;
    };

  public:
    WLibraryTableView(QWidget* parent,
            UserSettingsPointer pConfig);
    ~WLibraryTableView() override;

    void moveSelection(int delta) override;

    /// @brief saveTrackModelState function saves current positions of scrollbars,
    /// current item selection and current index in a QCache using a unique
    /// string key - can be any value but should invariant for model
    /// @param key unique for trackmodel
    void saveTrackModelState(const QAbstractItemModel* model, const QString& key);
    /// @brief restoreTrackModelState function finds scrollbar positions,
    /// item selection and current index values associated with model by given
    /// key and restores it
    /// @param key unique for trackmodel
    /// @return true if restore succeeded
    bool restoreTrackModelState(const QAbstractItemModel* model, const QString& key);
    void saveCurrentViewState() override;
    /// @brief restores current view state.
    /// @return true if restore succeeded
    bool restoreCurrentViewState() override;

  signals:
    void loadTrack(TrackPointer pTrack);
    void loadTrackToPlayer(TrackPointer pTrack, const QString& group, bool play = false);
    void trackSelected(TrackPointer pTrack);
    void onlyCachedCoverArt(bool);
    void scrollValueChanged(int);

  public slots:
    void setTrackTableFont(const QFont& font);
    void setTrackTableRowHeight(int rowHeight);
    void setSelectedClick(bool enable);

  protected:
    void focusInEvent(QFocusEvent* event) override;
    virtual QString getModelStateKey() const = 0;

  private:
    const UserSettingsPointer m_pConfig;
    QCache<QString, ModelState> m_modelStateCache;
};
