#pragma once
#include <QQuickItem>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QtGlobal>

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
  signals:
    Q_INVOKABLE void activated();

  private:
    QString m_label;
};

class QmlSettingParameterManager;
class QmlSettingParameter : public QmlSettingGroup {
    Q_OBJECT
    Q_PROPERTY(QStringList keywords MEMBER m_keywords FINAL)
    QML_NAMED_ELEMENT(SettingParameter)
    Q_INTERFACES(QQmlParserStatus)
  public:
    explicit QmlSettingParameter(QQuickItem* parent = nullptr);

    void componentComplete() override;

    const QStringList& keywords() const {
        return m_keywords;
    }

  private:
    QStringList m_keywords;
};

class QmlSettingParameterManager : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(QSortFilterProxyModel* model READ model CONSTANT)
    QML_NAMED_ELEMENT(SettingParameterManager)
  public:
    explicit QmlSettingParameterManager(QQuickItem* parent = nullptr);
    ~QmlSettingParameterManager();

    void registerSettingParamater(QmlSettingParameter* parameter, QList<QmlSettingGroup*>);
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
