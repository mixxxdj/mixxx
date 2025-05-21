#pragma once

#include <QCache>
#include <QString>
#include <QTableView>

#include "library/library_decl.h"
#include "library/libraryview.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"
#ifdef __STEM__
#include "engine/engine.h"
#endif

class QFont;

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
    /// @brief store the current index
    void saveCurrentIndex();
    /// @brief restores the current index, meant to provide a starting point for
    /// navigation after selected tracks have been manually removed from the view
    /// via hide, remove or purge
    /// @param optional: index, otherwise row/column member vars are used
    void restoreCurrentIndex(const QModelIndex& index = QModelIndex());

    void dataChanged(
            const QModelIndex& topLeft,
            const QModelIndex& bottomRight,
            const QVector<int>& roles = QVector<int>()) override;

  signals:
    void loadTrack(TrackPointer pTrack);
    void loadTrackToPlayer(TrackPointer pTrack,
            const QString& group,
#ifdef __STEM__
            mixxx::StemChannelSelection stemMask,
#endif
            bool play = false);
    void trackSelected(TrackPointer pTrack);
    void onlyCachedCoversAndOverviews(bool);
    void scrollValueChanged(int);
    FocusWidget setLibraryFocus(FocusWidget newFocus);

  public slots:
    void setTrackTableFont(const QFont& font);
    void setTrackTableRowHeight(int rowHeight);
    void setSelectedClick(bool enable);

  protected:
    void focusInEvent(QFocusEvent* event) override;
    QModelIndex moveCursor(CursorAction cursorAction,
            Qt::KeyboardModifiers modifiers) override;
    virtual QString getModelStateKey() const = 0;

    int m_prevRow;
    int m_prevColumn;

  private:
    const UserSettingsPointer m_pConfig;
    QCache<QString, ModelState> m_modelStateCache;
};
