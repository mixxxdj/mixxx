#include "tagging/taggingui.h"

#include <QCheckBox>
#include <QClipboard>
#include <QDoubleSpinBox>
#include <QGuiApplication>
#include <QInputDialog>
#include <QMimeData>
#include <QSignalBlocker>
#include <QStyledItemDelegate>
#include <QWidgetAction>
#include <memory>

#include "library/trackcollection.h"
#include "library/trackcollectioniterator.h"
#include "library/trackcollectionmanager.h"
#include "library/trackprocessing.h"
#include "tagging/taggingcontext.h"
#include "tagging/trackfacetsdb.h"
#include "track/track.h"
#include "util/logger.h"
#include "util/parented_ptr.h"
#include "util/qt.h"

namespace mixxx {

namespace {

const Logger kLogger("tagging.ui");

const QString kMimeTypeJson = QStringLiteral("application/json;charset=UTF-8");

class TrackMergeReplaceFacets : public TrackPointerOperation {
  public:
    TrackMergeReplaceFacets(
            const TrackCollectionManager* pTrackCollectionManager,
            const Facets& facets)
            : m_pTrackCollectionManager(pTrackCollectionManager),
              m_facets(facets) {
    }
    ~TrackMergeReplaceFacets() override = default;

  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        pTrack->mergeReplaceFacets(
                m_pTrackCollectionManager->taggingConfig(),
                m_facets);
    }

    const TrackCollectionManager* const m_pTrackCollectionManager;
    const Facets m_facets;
};

class TrackReplaceAllFacets : public TrackPointerOperation {
  public:
    TrackReplaceAllFacets(
            const TrackCollectionManager* pTrackCollectionManager,
            const Facets& facets)
            : m_pTrackCollectionManager(pTrackCollectionManager),
              m_facets(facets) {
    }
    ~TrackReplaceAllFacets() override = default;

  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        pTrack->updateFacets(
                m_pTrackCollectionManager->taggingConfig(),
                m_facets);
    }

    const TrackCollectionManager* const m_pTrackCollectionManager;
    const Facets m_facets;
};

class TrackReplaceCustomTag : public TrackPointerOperation {
  public:
    TrackReplaceCustomTag(
            const TrackCollectionManager* pTrackCollectionManager,
            const Tag& tag,
            const TagFacetId& facetId = TagFacetId{})
            : m_pTrackCollectionManager(pTrackCollectionManager),
              m_tag(tag),
              m_facetId(facetId) {
    }
    ~TrackReplaceCustomTag() override = default;

  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        pTrack->replaceCustomTag(
                m_pTrackCollectionManager->taggingConfig(),
                m_tag,
                m_facetId);
    }

    const TrackCollectionManager* const m_pTrackCollectionManager;
    const Tag m_tag;
    const TagFacetId m_facetId;
};

class TrackAppendCustomTag : public TrackPointerOperation {
  public:
    TrackAppendCustomTag(
            const TrackCollectionManager* pTrackCollectionManager,
            const TagLabel& newLabel,
            const TagFacetId& facetId)
            : m_pTrackCollectionManager(pTrackCollectionManager),
              m_newLabel(newLabel),
              m_facetId(facetId) {
    }
    ~TrackAppendCustomTag() override = default;

  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        pTrack->appendCustomTag(
                m_pTrackCollectionManager->taggingConfig(),
                m_newLabel,
                m_facetId);
    }

    const TrackCollectionManager* const m_pTrackCollectionManager;
    const TagLabel m_newLabel;
    const TagFacetId m_facetId;
};

class TrackRemoveCustomTag : public TrackPointerOperation {
  public:
    explicit TrackRemoveCustomTag(
            const TrackCollectionManager* pTrackCollectionManager,
            const TagLabel& label,
            const TagFacetId& facetId = TagFacetId{})
            : m_pTrackCollectionManager(pTrackCollectionManager),
              m_label(label),
              m_facetId(facetId) {
    }
    ~TrackRemoveCustomTag() override = default;

  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        pTrack->removeCustomTag(
                m_pTrackCollectionManager->taggingConfig(),
                m_label,
                m_facetId);
    }

    const TrackCollectionManager* const m_pTrackCollectionManager;
    const TagLabel m_label;
    const TagFacetId m_facetId;
};

std::optional<TagScore> parseTagScore(
        const QString& text) {
    bool ok;
    const auto scoreValue = text.toDouble(&ok);
    if (!ok) {
        return std::nullopt;
    }
    return TagScore(TagScore::convertIntoValidValue(scoreValue));
}

} // anonymous namespace

QString displayTagLabel(
        const TagLabel& label) {
    return label.value();
}

QString displayTagScore(
        const TagScore& score,
        const QLocale& locale) {
    return locale.toString(score.value());
}

QString displayTag(
        const Tag& tag,
        const QLocale& locale) {
    if (tag.hasLabel()) {
        // Only display the score if it differs from the default score
        if (tag.getScore() != TagScore()) {
            return QString(QStringLiteral("%1 (%2)"))
                    .arg(displayTagLabel(tag.getLabel()),
                            displayTagScore(tag.getScore(), locale));
        } else {
            // Omit the default score, show just the label
            return displayTagLabel(tag.getLabel());
        }
    } else {
        // Always display the score regardless of the value
        return displayTagScore(tag.getScore(), locale);
    }
}

TrackFacetsMenu::TrackFacetsMenu(
        const UserSettingsPointer& pSettings,
        TrackCollectionManager* pTrackCollectionManager,
        const QString& title,
        QWidget* parent)
        : QMenu(title, parent),
          m_pSettings(pSettings),
          m_pTrackCollectionManager(pTrackCollectionManager),
          m_storage(pTrackCollectionManager->internalCollection()->database()) {
}

void TrackFacetsMenu::rebuild(
        const TrackIdList& trackIds) {
    clear();

    m_trackIds = trackIds;
    if (m_trackIds.isEmpty()) {
        return;
    }

    {
        const auto allTags = m_pTrackCollectionManager->taggingConfig()
                                     .getFacets()
                                     .collectTags();
        for (const auto& tag : allTags) {
            addTagAction(this, tag);
        }
    }
    const parented_ptr<QAction> pAction(
            addAction(
                    tr("Add new tag..."),
                    this,
                    [this] {
                        const auto label = enterNewTagLabel();
                        if (label && !label->isEmpty()) {
                            onAddTag(
                                    Tag(*label));
                        }
                    }));
    pAction->setEnabled(!m_trackIds.isEmpty());
    addSeparator();
    for (auto i = m_pTrackCollectionManager->taggingConfig()
                          .getFacets()
                          .begin();
            i !=
            m_pTrackCollectionManager->taggingConfig()
                    .getFacets()
                    .end();
            ++i) {
        if (i.key().isEmpty()) {
            // Skip plain tags (see below)
            continue;
        }
        const auto& facetId = i.key();
        auto pSubMenu = parented_ptr(
                addMenu(m_pTrackCollectionManager->taggingContext().displayFacet(i.key())));
        for (auto j = i.value().begin();
                j != i.value().end();
                ++j) {
            const auto tag = Tag(j.key(), j.value());
            addTagAction(
                    pSubMenu,
                    tag,
                    facetId);
        }
        const parented_ptr<QAction> pAction(
                pSubMenu->addAction(
                        tr("Add new tag..."),
                        this,
                        [this, facetId] {
                            const auto label = enterNewTagLabel(facetId);
                            if (label && !label->isEmpty()) {
                                onAddTag(
                                        Tag(*label),
                                        facetId);
                            }
                        }));
        pAction->setEnabled(!m_trackIds.isEmpty());
    }
    addAction(
            tr("New facet..."),
            this,
            [this] {
                const auto facetId = enterNewTagFacetId();
                if (facetId && !facetId->isEmpty()) {
                    onNewTagFacetId(*facetId);
                }
            });
    addSeparator();
    const auto* pMimeData =
            getMimeDataForPastingFacetsFromClipboard();
    {
        const parented_ptr<QAction> pAction(
                addAction(
                        tr("Copy all custom tags"),
                        this,
                        [this] {
                            VERIFY_OR_DEBUG_ASSERT(m_trackIds.size() == 1) {
                                return;
                            }
                            const auto pTrack =
                                    m_pTrackCollectionManager->getTrackById(m_trackIds.first());
                            if (!pTrack) {
                                return;
                            }
                            copyFacetsToClipboard(pTrack->getFacets());
                        }));
        pAction->setEnabled(m_trackIds.size() == 1);
    }
    {
        const parented_ptr<QAction> pAction(
                addAction(
                        tr("Paste and merge into custom tags"),
                        this,
                        [this] {
                            setCursor(Qt::WaitCursor);
                            const auto facets =
                                    pasteFacetsFromClipboard();
                            if (!facets) {
                                return;
                            }
                            const auto trackOperator =
                                    TrackMergeReplaceFacets(
                                            m_pTrackCollectionManager,
                                            *facets);
                            // TODO: Ask user if multiple tracks are affected?
                            applyOperator(tr("Pasting and merging custom tags "
                                             "from clipboard into %1 track(s)")
                                                  .arg(QString::number(
                                                          m_trackIds.size())),
                                    &trackOperator);
                            unsetCursor();
                        }));
        pAction->setEnabled(!m_trackIds.isEmpty() && pMimeData);
    }
    {
        const parented_ptr<QAction> pAction(
                addAction(
                        tr("Paste and replace all custom tags"),
                        this,
                        [this] {
                            setCursor(Qt::WaitCursor);
                            const auto facets =
                                    pasteFacetsFromClipboard();
                            if (!facets) {
                                return;
                            }
                            const auto trackOperator =
                                    TrackReplaceAllFacets(
                                            m_pTrackCollectionManager,
                                            *facets);
                            // TODO: Ask user if multiple tracks are affected?
                            applyOperator(
                                    tr("Pasting and replacing all custom tags from "
                                       "clipboard into %1 track(s)")
                                            .arg(QString::number(
                                                    m_trackIds.size())),
                                    &trackOperator);
                            unsetCursor();
                        }));
        pAction->setEnabled(!m_trackIds.isEmpty() && pMimeData);
    }
    addSeparator();
    {
        const parented_ptr<QAction> pAction(
                addAction(
                        tr("Populate menu from selected tracks"),
                        this,
                        [this] {
                            setCursor(Qt::WaitCursor);
                            if (m_storage.mergeFacetsAndLabelsInto(
                                        m_pTrackCollectionManager->refTaggingContext()
                                                .refConfig()
                                                .ptrFacets(),
                                        m_trackIds) > 0) {
                                m_pTrackCollectionManager->taggingContext().saveConfigFile();
                            }
                            unsetCursor();
                        }));
        pAction->setEnabled(!m_trackIds.isEmpty());
    }
    {
        const parented_ptr<QAction> pAction(
                addAction(
                        tr("Populate menu from all tracks"),
                        this,
                        [this] {
                            setCursor(Qt::WaitCursor);
                            if (m_storage.mergeFacetsAndLabelsInto(
                                        m_pTrackCollectionManager->refTaggingContext()
                                                .refConfig()
                                                .ptrFacets()) > 0) {
                                m_pTrackCollectionManager->taggingContext().saveConfigFile();
                            }
                            unsetCursor();
                        }));
    }
}

//static
std::optional<TagFacetId> TrackFacetsMenu::enterNewTagFacetId() {
    bool ok = false;
    auto facetIdText =
            QInputDialog::getText(
                    nullptr,
                    tr("New Tag Facet"),
                    tr("Enter new facet identifier (%1):").arg(TagFacetId::kAlphabet),
                    QLineEdit::Normal,
                    QString(),
                    &ok)
                    .trimmed();
    if (!ok) {
        return std::nullopt;
    }
    const auto newFacetId = TagFacetId(
            TagFacetId::convertIntoValidValue(facetIdText));
    if (newFacetId.isEmpty()) {
        return newFacetId;
    }
    if (!newFacetId.isValid()) {
        QMessageBox(
                QMessageBox::Warning,
                tr("Invalid Tag Facet"),
                tr("'%1' is not a valid facet identifier.")
                        .arg(newFacetId),
                QMessageBox::Close)
                .exec();
        return std::nullopt;
    }
    if (isReservedFacetId(newFacetId)) {
        QMessageBox(
                QMessageBox::Warning,
                tr("Reserved Tag Facet"),
                tr("'%1' is reserved for internal usage and not available as a facet identifier.")
                        .arg(newFacetId),
                QMessageBox::Close)
                .exec();
        return std::nullopt;
    }
    return newFacetId;
}

//static
std::optional<TagLabel> TrackFacetsMenu::enterNewTagLabel(
        const QString& facetName) {
    bool ok = false;
    auto labelText =
            QInputDialog::getText(
                    nullptr,
                    tr("New Tag Label"),
                    facetName.isEmpty()
                            ? tr("Enter label for new tag:")
                            : tr("Enter label for new '%1' tag:")
                                      .arg(facetName),
                    QLineEdit::Normal,
                    QString(),
                    &ok)
                    .trimmed();
    if (!ok) {
        return std::nullopt;
    }
    const auto newLabel = TagLabel(
            TagLabel::convertIntoValidValue(labelText));
    if (newLabel.isEmpty()) {
        return newLabel;
    }
    if (!newLabel.isValid()) {
        QMessageBox(
                QMessageBox::Warning,
                tr("Invalid Tag Label"),
                tr("'%1' is not a valid tag label.")
                        .arg(newLabel),
                QMessageBox::Close)
                .exec();
        return std::nullopt;
    }
    return newLabel;
}

void TrackFacetsMenu::addTagAction(
        QMenu* parent,
        const Tag& tag,
        const TagFacetId& facetId) {
    auto pAction = make_parented<QWidgetAction>(
            parent);
    auto pCheckBox = make_parented<QCheckBox>(
            escapeTextPropertyWithoutShortcuts(displayTag(tag)),
            parent);
    pCheckBox->setShortcut(QKeySequence());
    if (m_trackIds.isEmpty()) {
        pCheckBox->setEnabled(false);
        pAction->setEnabled(false);
    } else {
        const int count = m_storage.countTags(
                m_trackIds,
                tag.getLabel(),
                facetId);
        DEBUG_ASSERT(count >= 0);
        DEBUG_ASSERT(count <= m_trackIds.size());
        if (count == 0) {
            pCheckBox->setChecked(false);
        } else if (count == m_trackIds.size()) {
            pCheckBox->setChecked(true);
        } else {
            pCheckBox->setTristate(true);
            pCheckBox->setCheckState(Qt::PartiallyChecked);
        }
        connect(
                pCheckBox,
                &QCheckBox::toggled,
                this,
                [pAction = pAction.get()] {
                    pAction->trigger();
                });
        connect(pAction,
                &QAction::triggered,
                this,
                [this, pCheckBox = pCheckBox.get(), tag, facetId] {
                    pCheckBox->setTristate(false);
                    if (pCheckBox->isChecked()) {
                        onAddTag(tag, facetId);
                    } else {
                        onRemoveTag(tag, facetId);
                    }
                });
    }
    pAction->setDefaultWidget(std::move(pCheckBox));
    parent->addAction(std::move(pAction));
}

void TrackFacetsMenu::onNewTagFacetId(
        const TagFacetId& facetId) {
    if (m_pTrackCollectionManager->taggingConfig()
                    .getFacets()
                    .containsFacet(facetId)) {
        kLogger.debug()
                << "Facet"
                << facetId
                << "already exists";
        return;
    }
    m_pTrackCollectionManager->refTaggingContext()
            .refConfig()
            .refFacets()
            .addOrIgnoreFacet(facetId);
    m_pTrackCollectionManager->taggingContext().saveConfigFile();
}

void TrackFacetsMenu::onAddTag(
        const Tag& tag,
        const TagFacetId& facetId) {
    if (!m_pTrackCollectionManager->taggingConfig().getFacets().containsTagLabeled(
                tag.getLabel(),
                facetId)) {
        m_pTrackCollectionManager->refTaggingContext()
                .refConfig()
                .refFacets()
                .addOrUpdateTag(tag, facetId);
        m_pTrackCollectionManager->taggingContext().saveConfigFile();
    }
    std::unique_ptr<TrackPointerOperation> pTrackOperator;
    if (facetId == library::tags::kFacetGenre ||
            facetId == library::tags::kFacetMood) {
        DEBUG_ASSERT(tag.getScore() == TagScore());
        pTrackOperator =
                std::make_unique<TrackAppendCustomTag>(
                        m_pTrackCollectionManager,
                        tag.getLabel(),
                        facetId);
    } else {
        pTrackOperator =
                std::make_unique<TrackReplaceCustomTag>(
                        m_pTrackCollectionManager,
                        tag,
                        facetId);
    }
    if (facetId.isEmpty()) {
        applyOperator(
                tr("Adding custom tag '%1' to %n track(s)", "", m_trackIds.size())
                        .arg(displayTag(tag)),
                pTrackOperator.get());
    } else {
        applyOperator(
                tr("Adding custom '%1' tag '%2' to %n track(s)", "", m_trackIds.size())
                        .arg(
                                m_pTrackCollectionManager->taggingContext().displayFacet(facetId),
                                displayTag(tag)),
                pTrackOperator.get());
    }
}

void TrackFacetsMenu::onRemoveTag(
        const Tag& tag,
        const TagFacetId& facetId) {
    const auto trackOperator =
            TrackRemoveCustomTag(
                    m_pTrackCollectionManager,
                    tag.getLabel(),
                    facetId);
    if (facetId.isEmpty()) {
        applyOperator(
                tr("Removing custom tag '%1' from %n track(s)", "", m_trackIds.size())
                        .arg(tag.getLabel()),
                &trackOperator);
    } else {
        applyOperator(
                tr("Removing custom '%1' tag '%2' from %n track(s)", "", m_trackIds.size())
                        .arg(
                                m_pTrackCollectionManager->taggingContext().displayFacet(facetId),
                                tag.getLabel()),
                &trackOperator);
    }
}

void TrackFacetsMenu::applyOperator(
        const QString& progressLabelText,
        const TrackPointerOperation* pTrackPointerOperation) {
    ModalTrackBatchOperationProcessor modalOperation(
            pTrackPointerOperation,
            // Explicitly save each track after modifying to update the
            // database with these changes. Otherwise those changes might
            // not be visible when opening the menu again with a selection
            // that contains recently modified tracks!
            ModalTrackBatchOperationProcessor::Mode::ApplyAndSave);
    auto iter = TrackByIdCollectionIterator(
            m_pTrackCollectionManager,
            m_trackIds);
    modalOperation.processTracks(
            progressLabelText,
            m_pTrackCollectionManager,
            &iter);
}

namespace {

class NoEditDelegate : public QStyledItemDelegate {
  public:
    explicit NoEditDelegate(QObject* parent = nullptr)
            : QStyledItemDelegate(parent) {
    }

    QWidget* createEditor(
            QWidget* parent,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override {
        Q_UNUSED(parent)
        Q_UNUSED(option)
        Q_UNUSED(index)
        return nullptr;
    }
};

class ScoreItemDelegate : public QStyledItemDelegate {
  public:
    explicit ScoreItemDelegate(
            QObject* parent = nullptr)
            : QStyledItemDelegate(parent) {
    }

    QWidget* createEditor(
            QWidget* parent,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override {
        Q_UNUSED(option)
        Q_UNUSED(index)
        const auto spinBox = make_parented<QDoubleSpinBox>(parent);
        spinBox->setRange(TagScore::kMinValue, TagScore::kMaxValue);
        spinBox->setSingleStep((TagScore::kMaxValue - TagScore::kMinValue) / 100);
        spinBox->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
        return spinBox.get();
    }
};

} // anonymous namespace

FacetsTreeWidgetHelper::FacetsTreeWidgetHelper(
        QTreeWidget* pTreeWidget)
        : QObject(pTreeWidget) {
    DEBUG_ASSERT(pTreeWidget);
    pTreeWidget->setRootIsDecorated(false);
    pTreeWidget->setUniformRowHeights(true);
    if (pTreeWidget->columnCount() != kColumnCount) {
        pTreeWidget->setColumnCount(kColumnCount);
        pTreeWidget->setColumnWidth(kFacetColumn, 120);
        pTreeWidget->setColumnWidth(kLabelColumn, 120);
        pTreeWidget->setColumnWidth(kScoreColumn, 40);
    }
    pTreeWidget->setHeaderLabels({
            tr("Facet"),
            tr("Label"),
            tr("Score"),
    });
    pTreeWidget->setSelectionMode(
            QAbstractItemView::ExtendedSelection);
    pTreeWidget->setEditTriggers(
            QAbstractItemView::EditTrigger::DoubleClicked |
            QAbstractItemView::EditTrigger::SelectedClicked |
            QAbstractItemView::EditTrigger::EditKeyPressed);
    pTreeWidget->setItemDelegateForColumn(
            kFacetColumn,
            new NoEditDelegate(pTreeWidget));
    pTreeWidget->setItemDelegateForColumn(
            kLabelColumn,
            new NoEditDelegate(pTreeWidget));
    pTreeWidget->setItemDelegateForColumn(
            kScoreColumn,
            new ScoreItemDelegate(pTreeWidget));
    const parented_ptr<QAction> pCutSelectionToClipboardAction(
            m_contextMenu.addAction(
                    tr("Cut selected tags"),
                    this,
                    [this, pTreeWidget] {
                        const auto exportedData = exportData(ExportData::OnlySelected);
                        if (exportedData) {
                            copyFacetsToClipboard(*exportedData);
                            removeSelectedItems(pTreeWidget);
                        }
                    },
                    QKeySequence(tr("Ctrl+X", "Edit|Cut"))));
    const parented_ptr<QAction> pCopySelectionToClipboardAction(
            m_contextMenu.addAction(
                    tr("Copy selected tags"),
                    this,
                    [this] {
                        const auto exportedData = exportData(ExportData::OnlySelected);
                        if (exportedData) {
                            copyFacetsToClipboard(*exportedData);
                        }
                    },
                    QKeySequence(tr("Ctrl+C", "Edit|Copy"))));
    const parented_ptr<QAction> pPasteAndMergeFromClipboardAction(
            m_contextMenu.addAction(
                    tr("Paste and merge"),
                    this,
                    [this] {
                        emit pasteAndMergeFromClipboard();
                    },
                    QKeySequence(tr("Ctrl+V", "Edit|Paste"))));
    m_contextMenu.addSeparator();
    const parented_ptr<QAction> pDeleteSelectionAction(
            m_contextMenu.addAction(
                    tr("Delete selected tags"),
                    this,
                    [this, pTreeWidget] {
                        removeSelectedItems(pTreeWidget);
                    }));
    pTreeWidget->setContextMenuPolicy(
            Qt::CustomContextMenu);
    connect(pTreeWidget,
            &QWidget::customContextMenuRequested,
            [this,
                    pTreeWidget,
                    pCutSelectionToClipboardAction = pCutSelectionToClipboardAction.get(),
                    pCopySelectionToClipboardAction = pCopySelectionToClipboardAction.get(),
                    pPasteAndMergeFromClipboardAction = pPasteAndMergeFromClipboardAction.get(),
                    pDeleteSelectionAction = pDeleteSelectionAction.get()](
                    const QPoint& pos) {
                const bool selectionEmpty = pTreeWidget->selectedItems().isEmpty();
                pCutSelectionToClipboardAction->setEnabled(
                        !selectionEmpty);
                pCopySelectionToClipboardAction->setEnabled(
                        !selectionEmpty);
                const auto* pMimeData =
                        mixxx::getMimeDataForPastingFacetsFromClipboard();
                pPasteAndMergeFromClipboardAction->setEnabled(
                        pMimeData != nullptr);
                pDeleteSelectionAction->setEnabled(
                        !selectionEmpty);
                m_contextMenu.popup(pTreeWidget->mapToGlobal(pos));
            });
}

void FacetsTreeWidgetHelper::importData(
        const TaggingContext& context,
        const Facets& facets) const {
    auto* const pTreeWidget = qobject_cast<QTreeWidget*>(parent());

    const QSignalBlocker signalBlocker(pTreeWidget);

    pTreeWidget->clear();
    for (auto i = facets.begin();
            i != facets.end();
            ++i) {
        const auto& facetId = i.key();
        TagVector tags;
        if (facetId.isEmpty()) {
            tags = facets.collectTags();
        } else {
            tags = facets.collectTagsOrdered(
                    Facets::ScoreOrdering::Descending,
                    facetId);
        }
        for (const auto& tag : qAsConst(tags)) {
            auto pItem = std::make_unique<QTreeWidgetItem>(pTreeWidget);
            pItem->setText(
                    kFacetColumn,
                    context.displayFacet(facetId));
            pItem->setData(
                    kFacetColumn,
                    Qt::UserRole,
                    facetId.value());
            pItem->setText(
                    kLabelColumn,
                    displayTagLabel(tag.getLabel()));
            pItem->setData(
                    kLabelColumn,
                    Qt::UserRole,
                    tag.getLabel().value());
            pItem->setText(
                    kScoreColumn,
                    displayTagScore(tag.getScore()));
            pItem->setData(
                    kScoreColumn,
                    Qt::EditRole,
                    tag.getScore().value());
            pItem->setFlags(Qt::ItemFlags(
                    Qt::ItemNeverHasChildren |
                    Qt::ItemIsEnabled |
                    Qt::ItemIsSelectable |
                    Qt::ItemIsEditable));
            pTreeWidget->addTopLevelItem(pItem.get());
            pItem.release();
        }
    }
}

namespace {

std::optional<std::pair<Tag, TagFacetId>> exportItemData(
        const QTreeWidgetItem& item) {
    const auto facetData = item.data(
            FacetsTreeWidgetHelper::kFacetColumn,
            Qt::UserRole);
    auto facetIdValue = qvariant_cast<TagFacetId::value_t>(facetData);
    VERIFY_OR_DEBUG_ASSERT(TagFacetId::isValidValue(facetIdValue)) {
        // Immutable and always valid
        kLogger.warning()
                << "Invalid facet value"
                << facetIdValue;
        return std::nullopt;
    }
    const auto labelData = item.data(
            FacetsTreeWidgetHelper::kLabelColumn,
            Qt::UserRole);
    const auto labelValue = qvariant_cast<TagLabel::value_t>(labelData);
    VERIFY_OR_DEBUG_ASSERT(TagLabel::isValidValue(labelValue)) {
        // Immutable and always valid
        kLogger.warning()
                << "Invalid label value"
                << labelValue;
        return std::nullopt;
    }
    const auto scoreData = item.data(
            FacetsTreeWidgetHelper::kScoreColumn,
            Qt::EditRole);
    const auto scoreValue = qvariant_cast<TagScore::value_t>(scoreData);
    VERIFY_OR_DEBUG_ASSERT(TagScore::isValidValue(scoreValue)) {
        // Already validated when edited and always valid
        kLogger.warning()
                << "Invalid score value"
                << scoreValue;
        return std::nullopt;
    }
    auto tag = Tag(
            TagLabel(labelValue),
            TagScore(scoreValue));
    auto facetId = TagFacetId(facetIdValue);
    return std::make_pair(tag, facetId);
}

} // anonymous namespace

std::optional<Facets> FacetsTreeWidgetHelper::exportData(
        ExportData exportData) const {
    auto* const pTreeWidget = qobject_cast<QTreeWidget*>(parent());
    Facets facets;
    for (auto i = 0; i < pTreeWidget->topLevelItemCount(); ++i) {
        const auto* pItem = pTreeWidget->topLevelItem(i);
        DEBUG_ASSERT(pItem);
        switch (exportData) {
        case ExportData::All:
            break;
        case ExportData::OnlySelected:
            if (!pItem->isSelected()) {
                // skip item
                continue;
            }
            break;
        default:
            DEBUG_ASSERT(!"invalid ExportOption");
        }
        auto exportedItemData = exportItemData(*pItem);
        VERIFY_OR_DEBUG_ASSERT(exportedItemData) {
            return std::nullopt;
        }
        auto [tag, facetId] = std::move(*exportedItemData);
        VERIFY_OR_DEBUG_ASSERT(!facets.containsTagLabeled(tag.getLabel(), facetId)) {
            if (facetId.isEmpty()) {
                kLogger.warning()
                        << "Duplicate tag label"
                        << tag.getLabel();
            } else {
                kLogger.warning()
                        << "Duplicate tag label"
                        << tag.getLabel()
                        << "for facet"
                        << facetId;
            }
            return std::nullopt;
        }
        facets.addOrUpdateTag(tag, facetId);
    }
    return facets;
}

void FacetsTreeWidgetHelper::removeSelectedItems(
        QTreeWidget* pTreeWidget) {
    VERIFY_OR_DEBUG_ASSERT(pTreeWidget) {
        return;
    }
    const auto selectedRanges = pTreeWidget->selectionModel()->selection().toVector();
    // Remove rows in reverse order to not invalidate preceding model indexes!
    for (auto i = selectedRanges.rbegin(); i != selectedRanges.rend(); ++i) {
        pTreeWidget->model()->removeRows(i->top(), i->height());
    }
    emit itemsRemoved();
}

bool FacetsTreeWidgetHelper::onItemChanged(
        Facets* pFacets,
        const QTreeWidgetItem& item,
        int column) const {
    VERIFY_OR_DEBUG_ASSERT(pFacets) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(column == kScoreColumn) {
        return false;
    }
    const auto scoreText = item.text(kScoreColumn);
    const auto score = parseTagScore(scoreText);
    if (!score) {
        kLogger.info()
                << "Failed to parse tag score"
                << scoreText;
        // Enforce an update to replace any invalid data
        return true;
    }
    DEBUG_ASSERT(score->isValid());
    const auto facetData = item.data(kFacetColumn, Qt::UserRole);
    auto facetIdValue = qvariant_cast<TagFacetId::value_t>(facetData);
    VERIFY_OR_DEBUG_ASSERT(TagFacetId::isValidValue(facetIdValue)) {
        return false;
    }
    const auto labelData = item.data(kLabelColumn, Qt::UserRole);
    const auto labelValue = qvariant_cast<TagLabel::value_t>(labelData);
    VERIFY_OR_DEBUG_ASSERT(TagLabel::isValidValue(labelValue)) {
        return false;
    }
    auto label = TagLabel(labelValue);
    auto facetId = TagFacetId(facetIdValue);
    return pFacets->addOrUpdateTag(
            Tag(label, *score),
            facetId);
}

bool FacetsTreeWidgetHelper::onItemsRemoved(
        Facets* pFacets) const {
    VERIFY_OR_DEBUG_ASSERT(pFacets) {
        return false;
    }
    auto exportedData = exportData(ExportData::All);
    VERIFY_OR_DEBUG_ASSERT(exportedData) {
        // Enforce an update to replace any invalid data
        return true;
    }
    if (*exportedData == *pFacets) {
        // Unmodified
        return false;
    }
    // Replace old with new data
    *pFacets = std::move(*exportedData);
    return true;
}

void copyFacetsToClipboard(
        const Facets& facets) {
    auto pMimeData = std::make_unique<QMimeData>();
    pMimeData->setData(kMimeTypeJson, facets.dumpJsonData());
    QGuiApplication::clipboard()->setMimeData(pMimeData.release());
}

const QMimeData* getMimeDataForPastingFacetsFromClipboard() {
    const auto* pMimeData = QGuiApplication::clipboard()->mimeData();
    if (pMimeData && pMimeData->hasFormat(kMimeTypeJson)) {
        return pMimeData;
    } else {
        return nullptr;
    }
}

std::optional<Facets> pasteFacetsFromClipboard() {
    const auto* pMimeData = getMimeDataForPastingFacetsFromClipboard();
    if (!pMimeData) {
        return std::nullopt;
    }
    const auto jsonData = pMimeData->data(kMimeTypeJson);
    // FIXME: Propagate errors?
    return Facets::parseJsonData(jsonData).first;
}

} // namespace mixxx
