#include "qml/qmlsettingparameter.h"

#include <qnamespace.h>

#include <QObject>

#include "moc_qmlsettingparameter.cpp"

namespace mixxx {
namespace qml {

QmlSettingGroup::QmlSettingGroup(QQuickItem* parent)
        : QQuickItem(parent) {
}
QmlSettingParameter::QmlSettingParameter(QQuickItem* parent)
        : QmlSettingGroup(parent) {
}

QmlSettingParameterManager::QmlSettingParameterManager(QObject* parent)
        : QObject(parent),
          m_model(new QSortFilterProxyModel(this)) {
    m_model.setSourceModel(&m_sourceModel);
    m_model.setFilterKeyColumn(1);
    m_model.setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void QmlSettingParameterManager::indexParameters(int, QQuickItem* view) {
    QStringList path;
    QList<QmlSettingGroup*> pathItems;
    for (auto* pParameter : view->findChildren<QmlSettingParameter*>()) {
        QObject* pParent = pParameter;
        path.clear();
        pathItems.clear();
        do {
            pParent = pParent->parent();
            auto* pGroup = qobject_cast<QmlSettingGroup*>(pParent);
            if (pGroup != nullptr) {
                path.prepend(pGroup->label());
                pathItems.prepend(pGroup);
            }
        } while (pParent && pParent != view);
        pathItems.append(pParameter);

        auto* pItem = new QStandardItem(pParameter->label());
        pItem->setData(path.join(" > "), Qt::WhatsThisRole);
        pItem->setData(QVariant::fromValue(pathItems), Qt::ToolTipRole);
        m_sourceModel.appendRow(QList<QStandardItem*>{pItem,
                new QStandardItem(pParameter->label() + path.join(" > "))});
    }
}
void QmlSettingParameterManager::search(const QString& criteria) {
    m_model.setFilterFixedString(criteria);
}

} // namespace qml
} // namespace mixxx
