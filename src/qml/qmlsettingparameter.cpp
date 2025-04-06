#include "qml/qmlsettingparameter.h"

#include <qnamespace.h>

#include <QObject>

#include "moc_qmlsettingparameter.cpp"
#include "util/assert.h"

namespace mixxx {
namespace qml {

QmlSettingGroup::QmlSettingGroup(QQuickItem* parent)
        : QQuickItem(parent) {
}
QmlSettingParameter::QmlSettingParameter(QQuickItem* parent)
        : QmlSettingGroup(parent),
          m_pManager(nullptr) {
}

QmlSettingParameter::~QmlSettingParameter() {
    if (m_pManager) {
        m_pManager->deregisterSettingParamater(this);
    }
}

void QmlSettingParameter::componentComplete() {
    QmlSettingGroup::componentComplete();
    QList<QmlSettingGroup*> pathItems;
    auto* pParent = parentItem();
    while (pParent != nullptr) {
        m_pManager = qobject_cast<QmlSettingParameterManager*>(pParent);
        if (m_pManager) {
            m_pManager->registerSettingParamater(this, pathItems);
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
          m_model(new QSortFilterProxyModel(this)) {
    m_model.setSourceModel(&m_sourceModel);
    m_model.setFilterKeyColumn(1);
    m_model.setFilterCaseSensitivity(Qt::CaseInsensitive);
}
QmlSettingParameterManager::~QmlSettingParameterManager() {
    // Manually deleting children so they can complete deregistration
    qDeleteAll(childItems());
}

void QmlSettingParameterManager::registerSettingParamater(QmlSettingParameter* pParameter, QList<QmlSettingGroup*> pathItems) {
    QStringList path;
    for (const auto* pItem : pathItems) {
        path.append(pItem->label());
    }
    pathItems.append(pParameter);

    auto* pItem = new QStandardItem(pParameter->label());
    pItem->setData(QVariant::fromValue(pParameter));
    pItem->setData(path.join(" > "), Qt::WhatsThisRole);
    pItem->setData(QVariant::fromValue(pathItems), Qt::ToolTipRole);
    m_sourceModel.appendRow(QList<QStandardItem*>{pItem,
            new QStandardItem(pParameter->label() + path.join(" > "))});
}

void QmlSettingParameterManager::deregisterSettingParamater(QmlSettingParameter* pParameter) {
    QList<QStandardItem*> pItems = m_sourceModel.findItems(pParameter->label());
    for (auto* pItem : std::as_const(pItems)) {
        // Handling duplicated items
        auto* pData = pItem->data().view<QmlSettingParameter*>();
        if (pData == pParameter) {
            m_sourceModel.removeRows(pItem->row(), 1);
            return;
        }
    }
    DEBUG_ASSERT(!"Couldn't find item!");
}
void QmlSettingParameterManager::search(const QString& criteria) {
    m_model.setFilterFixedString(criteria);
}

} // namespace qml
} // namespace mixxx
