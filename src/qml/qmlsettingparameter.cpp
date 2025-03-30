#include "qml/qmlsettingparameter.h"

#include <qnamespace.h>
#include <qobject.h>

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

void QmlSettingParameterManager::indexParameters(int categoryIdx, QQuickItem* view) {
    QStringList path;
    for (const auto& parameter : view->findChildren<QmlSettingParameter*>()) {
        QObject* pParent = parameter;
        path.clear();
        do {
            pParent = pParent->parent();
            auto* pGroup = qobject_cast<QmlSettingGroup*>(pParent);
            if (pGroup != nullptr) {
                path.prepend(pGroup->label());
            }
        } while (pParent && pParent != view);

        auto* pItem = new QStandardItem(parameter->label());
        pItem->setData(path.join(" > "), Qt::WhatsThisRole);
        pItem->setData(categoryIdx, Qt::ToolTipRole);
        m_sourceModel.appendRow(QList<QStandardItem*>{pItem,
                new QStandardItem(parameter->label() + path.join(" > "))});
    }
}
void QmlSettingParameterManager::search(const QString& criteria) {
    m_model.setFilterFixedString(criteria);
}

} // namespace qml
} // namespace mixxx
