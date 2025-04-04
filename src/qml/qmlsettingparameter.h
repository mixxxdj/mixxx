#pragma once
#include <qglobal.h>
#include <qquickitem.h>
#include <qsortfilterproxymodel.h>
#include <qstandarditemmodel.h>

#include <QQuickItem>

namespace mixxx {
namespace qml {

class QmlSettingGroup : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(QString label MEMBER m_label FINAL)
    QML_NAMED_ELEMENT(SettingGroup)
  public:
    explicit QmlSettingGroup(QQuickItem* parent = nullptr);

    const QString& label() const {
        return m_label;
    }

  private:
    QString m_label;
};

class QmlSettingParameter : public QmlSettingGroup {
    Q_OBJECT
    Q_PROPERTY(QStringList keywords MEMBER m_keywords FINAL)
    QML_NAMED_ELEMENT(SettingParameter)
  public:
    explicit QmlSettingParameter(QQuickItem* parent = nullptr);

    const QStringList& keywords() const {
        return m_keywords;
    }

  private:
    QStringList m_keywords;
};

class QmlSettingParameterManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QSortFilterProxyModel* model READ model CONSTANT)
    QML_NAMED_ELEMENT(SettingParameterManager)
  public:
    explicit QmlSettingParameterManager(QObject* parent = nullptr);

    Q_INVOKABLE void indexParameters(int categoryIdx, QQuickItem* view);
    Q_INVOKABLE void search(const QString& criteria);

    QSortFilterProxyModel* model() {
        return &m_model;
    }

  private:
    QStandardItemModel m_sourceModel;
    QSortFilterProxyModel m_model;
};

} // namespace qml
} // namespace mixxx
