#include "preferences/effectmanifesttablemodel.h"

#include <QMimeData>

#include "effects/backends/effectmanifest.h"
#include "effects/backends/effectsbackend.h"
#include "effects/backends/effectsbackendmanager.h"
#include "moc_effectmanifesttablemodel.cpp"

namespace {
const int kColumnType = 0;
const int kColumnName = 1;
const int kNumberOfColumns = 2;
constexpr QChar kMimeTextDelimiter('\n');
const QStringList kAcceptedMimeTypes = {QStringLiteral("text/plain")};
} // anonymous namespace

EffectManifestTableModel::EffectManifestTableModel(QObject* parent,
        EffectsBackendManagerPointer pBackendManager)
        : QAbstractTableModel(parent),
          m_pBackendManager(pBackendManager) {
}

void EffectManifestTableModel::setList(const QList<EffectManifestPointer>& newList) {
    removeRows(0, m_manifests.size());
    beginInsertRows(QModelIndex(), 0, newList.size() - 1);
    m_manifests = newList;
    endInsertRows();
}

int EffectManifestTableModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return m_manifests.size();
}

int EffectManifestTableModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return kNumberOfColumns;
}

QVariant EffectManifestTableModel::headerData(
        int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            if (section == kColumnType) {
                return tr("Type");
            } else if (section == kColumnName) {
                return tr("Name");
            }
        }
    }
    return QVariant();
}

QVariant EffectManifestTableModel::data(
        const QModelIndex& index, int role) const {
    int rowIndex = index.row();
    if (!index.isValid() || rowIndex >= m_manifests.size()) {
        return QVariant();
    }

    EffectManifestPointer pManifest = m_manifests.at(rowIndex);
    VERIFY_OR_DEBUG_ASSERT(pManifest) {
        return QVariant();
    }
    int column = index.column();
    if (column == kColumnName && role == Qt::DisplayRole) {
        return pManifest->displayName();
    } else if (column == kColumnType && role == Qt::DisplayRole) {
        return EffectsBackend::translatedBackendName(pManifest->backendType());
    }
    return QVariant();
}

Qt::ItemFlags EffectManifestTableModel::flags(const QModelIndex& index) const {
    return QAbstractTableModel::flags(index) | Qt::ItemIsSelectable |
            Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

QMimeData* EffectManifestTableModel::mimeData(
        const QModelIndexList& indexes) const {
    QMimeData* mimeData = new QMimeData();
    QStringList manifestUniqueIds;

    for (const auto& index : indexes) {
        if (!index.isValid() || index.row() < 0 ||
                index.row() >= m_manifests.size()
                // QModelIndexes for both columns are dragged, but only one of them in each row is needed.
                || index.column() != 0) {
            continue;
        }
        EffectManifestPointer pManifest = m_manifests.at(index.row());
        manifestUniqueIds << pManifest->uniqueId();
    }
    mimeData->setText(manifestUniqueIds.join(kMimeTextDelimiter));
    return mimeData;
}

bool EffectManifestTableModel::dropMimeData(const QMimeData* data,
        Qt::DropAction action,
        int row,
        int column,
        const QModelIndex& parent) {
    Q_UNUSED(column);
    if (!data->hasText() || action != Qt::MoveAction) {
        return false;
    }
    QStringList mimeTextLines =
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            data->text().split(kMimeTextDelimiter, Qt::SkipEmptyParts);
#else
            data->text().split(kMimeTextDelimiter, QString::SkipEmptyParts);
#endif
    QVector<EffectManifestPointer> manifestList;
    for (int lineNumber = 0; lineNumber < mimeTextLines.size(); lineNumber++) {
        QString manifestUniqueId = mimeTextLines.at(lineNumber);
        EffectManifestPointer pManifest =
                m_pBackendManager->getManifestFromUniqueId(manifestUniqueId);
        // Do not VERIFY_OR_DEBUG_ASSERT here because MIME data could come from anywhere.
        if (pManifest) {
            manifestList.append(pManifest);
        }
    }
    if (manifestList.isEmpty()) {
        return false;
    }
    if (row == -1) {
        if (parent.isValid()) {
            row = parent.row();
        }
        // Dropping onto an empty model or dropping past the end of a model
        if (row == -1) {
            row = m_manifests.size();
        }
    }
    beginInsertRows(QModelIndex(), row, row + manifestList.size() - 1);
    for (int manifestNumber = 0; manifestNumber < manifestList.size(); manifestNumber++) {
        m_manifests.insert(row + manifestNumber, manifestList.at(manifestNumber));
    }
    endInsertRows();
    return true;
}

QStringList EffectManifestTableModel::mimeTypes() const {
    return kAcceptedMimeTypes;
}

Qt::DropActions EffectManifestTableModel::supportedDropActions() const {
    return Qt::MoveAction;
}

bool EffectManifestTableModel::removeRows(int row, int count, const QModelIndex& parent) {
    if (count == 0) {
        // nothing to do
        return true;
    }
    VERIFY_OR_DEBUG_ASSERT(count > 0 && row >= 0 &&
            row + count <= m_manifests.size()) {
        // If this is violated, Mixxx will crash with a qt_assert()
        // https://github.com/mixxxdj/mixxx/issues/11454
        return false;
    }
    beginRemoveRows(parent, row, row + count - 1);
    for (int i = row; i < row + count; i++) {
        // QList shrinks and reassigns indices after each removal,
        // so keep removing at the index of the first row removed.
        m_manifests.removeAt(row);
    }
    endRemoveRows();
    return true;
}
