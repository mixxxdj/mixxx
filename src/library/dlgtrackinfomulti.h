#pragma once

#include <QDialog>
#include <QHash>
#include <QModelIndex>
#include <memory>

#include "library/ui_dlgtrackinfomulti.h"
#include "preferences/usersettings.h"
#include "track/beats.h"
#include "track/track_decl.h"
#include "track/trackrecord.h"
#include "util/parented_ptr.h"
#include "util/tapfilter.h"
#include "widget/wcolorpickeraction.h"

class WColorPickerAction;
class WStarRating;
class WCoverArtMenu;
class WCoverArtLabel;

/// A dialog box to display and edit properties of multiple tracks.
/// Use TrackPointers to load a track into the dialog.
/// Only invoked from WTrackTbelView's WTrackMenu.
class DlgTrackInfoMulti : public QDialog, public Ui::DlgTrackInfoMulti {
    Q_OBJECT
  public:
    explicit DlgTrackInfoMulti(UserSettingsPointer pUserSettings);
    ~DlgTrackInfoMulti() override = default;

    void loadTracks(const QList<TrackPointer>& pTracks);

  private slots:
    void slotOk();
    void slotApply();
    void slotCancel();

    void slotImportMetadataFromFiles();

    /// If only one track is changed while the dialog is open, re-populate
    /// the dialog from all tracks. This discards pending changes.
    void slotTrackChanged(TrackId trackId);

    void slotColorButtonClicked();
    void slotColorPicked(const mixxx::RgbColor::optional_t& newColor);

    void slotStarRatingChanged(int rating);

    void slotCoverFound(
            const QObject* pRequester,
            const CoverInfo& info,
            const QPixmap& pixmap);
    void slotCoverInfoSelected(const CoverInfoRelative& coverInfo);

    void slotReloadCoverArt();

  private:
    void init();
    void loadTracksInternal(const QList<TrackPointer>& pTracks);
    void saveTracks();
    void clear();

    void connectTracksChanged();
    void disconnectTracksChanged();

    void updateFromTracks();
    void replaceTrackRecords(const QList<mixxx::TrackRecord>& trackRecords);

    void updateTrackMetadataFields();
    void addValuesToComboBox(
            QComboBox* pBox,
            QStringList& values,
            bool sort = false);
    void updateCoverArtFromTracks();
    void trackColorDialogSetColorStyleButton(const mixxx::RgbColor::optional_t& color,
            bool variousColors = false);

    TrackPointer getTrackFromSetById(TrackId trackId) {
        DEBUG_ASSERT(!m_pLoadedTracks.isEmpty());
        auto trackIt = m_pLoadedTracks.constFind(trackId);
        VERIFY_OR_DEBUG_ASSERT(trackIt != m_pLoadedTracks.constEnd()) {
            return {};
        }
        return trackIt.value();
    }

    const UserSettingsPointer m_pUserSettings;

    QHash<TrackId, TrackPointer> m_pLoadedTracks;
    QList<mixxx::TrackRecord> m_trackRecords;

    parented_ptr<WCoverArtMenu> m_pWCoverArtMenu;
    parented_ptr<WCoverArtLabel> m_pWCoverArtLabel;
    parented_ptr<WStarRating> m_pWStarRating;
    bool m_starRatingModified;
    int m_newRating;
    bool m_colorChanged;
    mixxx::RgbColor::optional_t m_newColor;
    parented_ptr<WColorPickerAction> m_pColorPicker;
};
