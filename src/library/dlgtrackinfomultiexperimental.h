#pragma once

#include <QDialog>
#include <QHash>
#include <QModelIndex>
#include <QSet>
#include <QStringList>
#include <memory>

#include "library/ui_dlgtrackinfomultiexperimental.h"
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
class GenreDao;

class QScrollArea;
class QHBoxLayout;
class QWidget;

/// A dialog box to display and edit properties of multiple tracks.
/// Use TrackPointers to load a track into the dialog.
/// Only invoked from WTrackTbelView's WTrackMenu.
class DlgTrackInfoMultiExperimental : public QDialog, public Ui::DlgTrackInfoMultiExperimental {
    Q_OBJECT
  public:
    explicit DlgTrackInfoMultiExperimental(
            UserSettingsPointer pUserSettings,
            GenreDao& genreDao);
    ~DlgTrackInfoMultiExperimental() override = default;

    void loadTracks(const QList<TrackPointer>& pTracks);
    void focusField(const QString& property);

    void setGenreData(const QVariantList& genreData);
    void setupGenreCompleter();

  protected:
    /// We need this to set the max width of the comment QComboBox which has
    /// issues with long lines / multi-line content. See init() for details.
    /// Also used to set the maximum size of the cover label
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* pObj, QEvent* pEvent) override;

  private slots:
    void slotOk();
    void slotApply();
    void slotCancel();

    void slotImportMetadataFromFiles();

    /// If any of the loaded track has been changed while the dialog is open we
    /// re-populate the dialog from all tracks. This discards pending changes.
    void slotTrackChanged(TrackId trackId);

    void slotTagBoxIndexChanged();
    void slotCommentBoxIndexChanged();
    void commentTextChanged();
    void slotEditingFinished(QComboBox* pBox, QLineEdit* pLine);
    void slotKeyTextChanged();

    void slotColorButtonClicked();
    void slotColorPicked(const mixxx::RgbColor::optional_t& newColor);

    void slotStarRatingChanged(int rating);

    void slotCoverFound(
            const QObject* pRequester,
            const CoverInfo& info,
            const QPixmap& pixmap);
    void slotCoverInfoSelected(const CoverInfoRelative& coverInfo);
    void slotReloadCoverArt();

    void slotOpenInFileBrowser();

  private:
    void init();
    void loadTracksInternal(const QList<TrackPointer>& pTracks);
    void saveTracks();

    void connectTracksChanged();
    void disconnectTracksChanged();

    void updateFromTracks();
    void replaceTrackRecords(const QList<mixxx::TrackRecord>& trackRecords);

    void updateTrackMetadataFields();
    template<typename T>
    void addValuesToComboBox(
            QComboBox* pBox,
            QSet<T>& values,
            bool sort = false);
    void addValuesToCommentBox(QSet<QString>& comments);
    void updateTagPlaceholder(QComboBox* pBox, bool dirty);
    void updateCommentPlaceholder(bool dirty);

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
    GenreDao& m_genreDao;
    QHash<TrackId, TrackPointer> m_pLoadedTracks;
    QList<mixxx::TrackRecord> m_trackRecords;

    QHash<QString, QWidget*> m_propertyWidgets;

    // UI container
    QScrollArea* m_genreTagsArea = nullptr;
    QWidget* m_genreTagsContainer = nullptr;
    QHBoxLayout* m_genreTagsLayout = nullptr;

    // Intersection
    QStringList m_genreTagNames;
    QSet<QString> m_genreSeenLower;

    QSet<QString> m_pendingAdd;
    QSet<QString> m_pendingRemove;

    // Inline Genre Tags UI
    void genreTagsInitUi();
    void genreSetTags(const QStringList& names);
    QWidget* genreCreateChip(const QString& name);
    void genreRebuildChips();
    void genreAddTag(const QString& name);
    void genreRemoveTag(const QString& name);

    parented_ptr<WCoverArtMenu> m_pWCoverArtMenu;
    parented_ptr<WCoverArtLabel> m_pWCoverArtLabel;
    parented_ptr<WStarRating> m_pWStarRating;
    bool m_starRatingModified;
    int m_newRating;
    bool m_colorChanged;
    mixxx::RgbColor::optional_t m_newColor;
    parented_ptr<WColorPickerAction> m_pColorPicker;
    QVariantList m_genreData;
    QString m_rawGenreString;
};
