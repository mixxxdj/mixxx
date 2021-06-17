#pragma once

#include <QLocale>
#include <QMenu>
#include <QTreeWidget>

#include "preferences/usersettings.h"
#include "tagging/customtagsdb.h"
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

class TrackCustomTagsMenu : public QMenu {
    Q_OBJECT
  public:
    TrackCustomTagsMenu(
            const UserSettingsPointer& pSettings,
            TrackCollectionManager* pTrackCollectionManager,
            const QString& title,
            QWidget* parent = nullptr);
    ~TrackCustomTagsMenu() override = default;

    void rebuild(
            const TrackIdList& trackIds = TrackIdList{});

    QString displayFacet(const TagFacet& facet) const;

  private:
    static std::optional<TagFacet> enterNewTagFacet();
    static std::optional<TagLabel> enterNewTagLabel(
            const QString& facetName = QString());

    void addTagAction(
            QMenu* parent,
            const Tag& tag,
            const TagFacet& facet = TagFacet());

    void onNewTagFacet(
            const TagFacet& facet);
    void onAddTag(
            const Tag& tag,
            const TagFacet& facet = TagFacet());
    void onRemoveTag(
            const Tag& tag,
            const TagFacet& facet = TagFacet());

    void applyOperator(
            const QString& progressLabelText,
            const TrackPointerOperation* pTrackPointerOperation);

    const UserSettingsPointer m_pSettings;

    TrackCollectionManager* const m_pTrackCollectionManager;

    const TrackCustomTagsStorage m_storage;

    TrackIdList m_trackIds;
};

/// Wraps a QTreeWidget for displaying tags in a table.
class CustomTagsTreeWidgetHelper : public QObject {
    Q_OBJECT

  public:
    explicit CustomTagsTreeWidgetHelper(
            QTreeWidget* pTreeWidget);
    ~CustomTagsTreeWidgetHelper() override = default;

    static const int kFacetColumn = 0;
    static const int kLabelColumn = 1;
    static const int kScoreColumn = 2;
    static const int kColumnCount = 3;

    void importData(
            const TaggingContext& context,
            const CustomTags& customTags) const;

    bool onItemChanged(
            CustomTags* pCustomTags,
            const QTreeWidgetItem& item,
            int column) const;
    bool onItemsRemoved(
            CustomTags* pCustomTags) const;

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
    std::optional<CustomTags> exportData(
            ExportData exportData) const;

    QMenu m_contextMenu;
};

void copyCustomTagsToClipboard(
        const CustomTags& customTags);

/// Checks if the clipboard may contain pastable data.
/// May return false positives, i.e. favors responsiveness
/// over correctness.
const QMimeData* getMimeDataForPastingCustomTagsFromClipboard();

std::optional<CustomTags> pasteCustomTagsFromClipboard();

} // namespace mixxx
