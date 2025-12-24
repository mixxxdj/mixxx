#include "qml/qmlcuesmodel.h"

#include <QModelIndex>

#include "moc_qmlcuesmodel.cpp"
#include "track/cue.h"

namespace mixxx {
namespace qml {
namespace {
const QHash<int, QByteArray> kRoleNames = {
        {QmlCuesModel::StartPositionRole, "startPosition"},
        {QmlCuesModel::EndPositionRole, "endPosition"},
        {QmlCuesModel::LabelRole, "label"},
        {QmlCuesModel::IsLoopRole, "isLoop"},
        {QmlCuesModel::HotcueNumberRole, "hotcueNumber"},
};
}

QmlCuesModel::QmlCuesModel(
        QObject* pParent)
        : QAbstractListModel(pParent) {
}

void QmlCuesModel::setCues(QList<CuePointer> cues) {
    beginResetModel();
    m_cues = QList<CuePointer>(std::move(cues));
    endResetModel();
}

QVariant QmlCuesModel::data(const QModelIndex& index, int role) const {
    if (index.row() < 0 || index.row() >= m_cues.size()) {
        return QVariant();
    }

    const CuePointer& pCue = m_cues.at(index.row());
    VERIFY_OR_DEBUG_ASSERT(pCue.get()) {
        return QVariant();
    }

    switch (role) {
    case QmlCuesModel::StartPositionRole: {
        const auto position = pCue->getPosition();
        return position.isValid() ? position.value() : QVariant();
    }
    case QmlCuesModel::EndPositionRole: {
        const auto position = pCue->getEndPosition();
        return position.isValid() ? position.value() : QVariant();
    }
    case QmlCuesModel::LabelRole:
        return pCue->getLabel();
    case QmlCuesModel::IsLoopRole:
        return pCue->getType() == CueType::Loop;
    case QmlCuesModel::HotcueNumberRole:
        return pCue->getHotCue();
    default:
        return QVariant();
    }
}

int QmlCuesModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return m_cues.size();
}

QHash<int, QByteArray> QmlCuesModel::roleNames() const {
    return kRoleNames;
}

QVariant QmlCuesModel::get(int row) const {
    QModelIndex idx = index(row, 0);
    QVariantMap dataMap;
    for (auto it = kRoleNames.constBegin(); it != kRoleNames.constEnd(); it++) {
        dataMap.insert(it.value(), data(idx, it.key()));
    }
    return dataMap;
}

} // namespace qml
} // namespace mixxx
