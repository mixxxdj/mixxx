#include "qml/qmlsettingparameter.h"

#include <QObject>
#include <Qt>

#include "moc_qmlsettingparameter.cpp"
#include "util/assert.h"

namespace mixxx {
namespace qml {

QmlSettingGroup::QmlSettingGroup(QQuickItem* parent)
        : QQuickItem(parent) {
}
QmlSettingParameter::QmlSettingParameter(QQuickItem* parent)
        : QmlSettingGroup(parent) {
}

void QmlSettingParameter::componentComplete() {
    QmlSettingGroup::componentComplete();
    QList<QmlSettingGroup*> pathItems;
    auto* pParent = parentItem();
    while (pParent != nullptr) {
        auto* pManager = qobject_cast<QmlSettingParameterManager*>(pParent);
        if (pManager) {
            pManager->registerSettingParamater(this, pathItems);
            return;
        }
        auto* pGroup = qobject_cast<QmlSettingGroup*>(pParent);
        if (pGroup) {
            pathItems.prepend(pGroup);
        }
        pParent = pParent->parentItem();
    }
    DEBUG_ASSERT(!"Couldn't find manager!");
}

QmlSettingParameterManager::QmlSettingParameterManager(QQuickItem* parent)
        : QQuickItem(parent),
          m_model(this) {
    m_model.setSourceModel(&m_sourceModel);
    m_model.setFilterKeyColumn(1);
    m_model.setFilterCaseSensitivity(Qt::CaseInsensitive);
}
QmlSettingParameterManager::~QmlSettingParameterManager() {
    // Manually deleting children so they can complete deregistration
    qDeleteAll(childItems());
}

void QmlSettingParameterManager::registerSettingParamater(
        QmlSettingParameter* pParameter, QList<QmlSettingGroup*> pathItems) {
    QStringList path;
    for (const auto* pItem : pathItems) {
        path.append(pItem->label());
    }
    pathItems.append(pParameter);

    auto* pItem = new QStandardItem(pParameter->label());
    pItem->setData(path.join(" > "), Qt::WhatsThisRole);
    pItem->setData(QVariant::fromValue(pathItems), Qt::ToolTipRole);
    m_sourceModel.appendRow(QList<QStandardItem*>{pItem,
            new QStandardItem(pParameter->label() + path.join(" > "))});
    auto rowIndex = m_sourceModel.index(m_sourceModel.rowCount() - 1, 0);
    connect(pParameter, &QObject::destroyed, this, [this, rowIndex](QObject*) {
        m_sourceModel.removeRow(rowIndex.row());
    });
}

void QmlSettingParameterManager::search(const QString& criteria) {
    m_model.setFilterFixedString(criteria);
}

} // namespace qml
} // namespace mixxx
