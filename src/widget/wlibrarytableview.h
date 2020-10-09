// wlibrarytableview.h
// Created 10/19/2009 by RJ Ryan (rryan@mit.edu)

#ifndef WLIBRARYTABLEVIEW_H
#define WLIBRARYTABLEVIEW_H

#include <QFont>
#include <QString>
#include <QTableView>

#include "library/libraryview.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"

class TrackModel;

class WLibraryTableView : public QTableView, public virtual LibraryView {
    Q_OBJECT

  public:
    WLibraryTableView(QWidget* parent,
                      UserSettingsPointer pConfig,
                      ConfigKey vScrollBarPosKey);
    ~WLibraryTableView() override;

    void moveSelection(int delta) override;

    /**
     * @brief saveVScrollBarPos function saves current position of scrollbar
     * using string key - can be any value but should invariant for model
     * @param key unique for trackmodel
     */
    void saveVScrollBarPos(TrackModel* key);
    /**
     * @brief restoreVScrollBarPos function finds scrollbar value associated with model
     * by given key and restores it
     * @param key unique for trackmodel
     */
    void restoreVScrollBarPos(TrackModel* key);

  signals:
    void loadTrack(TrackPointer pTrack);
    void loadTrackToPlayer(TrackPointer pTrack, QString group,
            bool play = false);
    void trackSelected(TrackPointer pTrack);
    void onlyCachedCoverArt(bool);
    void scrollValueChanged(int);

  public slots:
    void setTrackTableFont(const QFont& font);
    void setTrackTableRowHeight(int rowHeight);
    void setSelectedClick(bool enable);

  protected:
    void focusInEvent(QFocusEvent* event) override;

    void saveNoSearchVScrollBarPos();
    void restoreNoSearchVScrollBarPos();

  private:
    void loadVScrollBarPosState();
    void saveVScrollBarPosState();

    const UserSettingsPointer m_pConfig;
    const ConfigKey m_vScrollBarPosKey;

    QMap<TrackModel*, int> m_vScrollBarPosValues;

    // The position of the vertical scrollbar slider, eg. before a search is
    // executed
    int m_noSearchVScrollBarPos;
};


#endif /* WLIBRARYTABLEVIEW_H */
