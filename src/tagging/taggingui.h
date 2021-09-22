#pragma once

#include <QLocale>
#include <QMenu>
#include <QTreeWidget>

#include "preferences/usersettings.h"
#include "tagging/trackfacetsdb.h"
#include "track/trackid.h"

class TrackCollectionManager;

namespace mixxx {

class TaggingContext;
class TrackPointerOperation;

QString displayTagLabel(
        const TagLabel& label);

QString displayTagScore(
        const TagScore& score,
        const QLocale& locale = QLocale());

/// Formats a plain tag as a single string for display.
QString displayTag(
        const Tag& tag,
        const QLocale& locale = QLocale());

class TrackFacetsMenu : public QMenu {
    Q_OBJECT
  public:
    TrackFacetsMenu(
            const UserSettingsPointer& pSettings,
            TrackCollectionManager* pTrackCollectionManager,
            const QString& title,
            QWidget* parent = nullptr);
    ~TrackFacetsMenu() override = default;

    void rebuild(
            const TrackIdList& trackIds = TrackIdList{});

    QString displayFacet(const TagFacetId& facetId) const;

  private:
    static std::optional<TagFacetId> enterNewTagFacetId();
    static std::optional<TagLabel> enterNewTagLabel(
            const QString& facetName = QString());

    void addTagAction(
            QMenu* parent,
            const Tag& tag,
            const TagFacetId& facetId = TagFacetId{});

    void onNewTagFacetId(
            const TagFacetId& facetId);
    void onAddTag(
            const Tag& tag,
            const TagFacetId& facetId = TagFacetId{});
    void onRemoveTag(
            const Tag& tag,
            const TagFacetId& facetId = TagFacetId{});

    void applyOperator(
            const QString& progressLabelText,
            const TrackPointerOperation* pTrackPointerOperation);

    const UserSettingsPointer m_pSettings;

    TrackCollectionManager* const m_pTrackCollectionManager;

    const TrackFacetsStorage m_storage;

    TrackIdList m_trackIds;
};

/// Wraps a QTreeWidget for displaying tags in a table.
class FacetsTreeWidgetHelper : public QObject {
    Q_OBJECT

  public:
    explicit FacetsTreeWidgetHelper(
            QTreeWidget* pTreeWidget);
    ~FacetsTreeWidgetHelper() override = default;

    static const int kFacetColumn = 0;
    static const int kLabelColumn = 1;
    static const int kScoreColumn = 2;
    static const int kColumnCount = 3;

    void importData(
            const TaggingContext& context,
            const Facets& facets) const;

    bool onItemChanged(
            Facets* pFacets,
            const QTreeWidgetItem& item,
            int column) const;
    bool onItemsRemoved(
            Facets* pFacets) const;

  signals:
    void pasteAndMergeFromClipboard();
    void itemsRemoved();

  private:
    void removeSelectedItems(
            QTreeWidget* pTreeWidget);

    enum class ExportData {
        All,
        OnlySelected,
    };
    std::optional<Facets> exportData(
            ExportData exportData) const;

    QMenu m_contextMenu;
};

void copyFacetsToClipboard(
        const Facets& facets);

/// Checks if the clipboard may contain pastable data.
/// May return false positives, i.e. favors responsiveness
/// over correctness.
const QMimeData* getMimeDataForPastingFacetsFromClipboard();

std::optional<Facets> pasteFacetsFromClipboard();

} // namespace mixxx
