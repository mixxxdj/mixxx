#pragma once
#include <QIdentityProxyModel>
#include <QQmlEngine>

class SidebarModel;

namespace mixxx {
namespace qml {

class QmlLibraryTreeviewModel : public QIdentityProxyModel {
    Q_OBJECT
    QML_NAMED_ELEMENT(LibraryTreeviewModel)
    QML_UNCREATABLE("Only accessible via Mixxx.Library.sidebar")

  public:
    enum Roles {
        LabelRole = Qt::UserRole,
        IconRole,
    };
    Q_ENUM(Roles);

    QmlLibraryTreeviewModel(SidebarModel* pModel, QObject* pParent = nullptr);
    ~QmlLibraryTreeviewModel() = default;

    QVariant data(const QModelIndex& index, int role) const override;
    QModelIndex index(int row,
            int column,
            const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int columnCount(const QModelIndex& index = QModelIndex()) const override;
    int rowCount(const QModelIndex& index = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE QVariant get(int row) const;
};

} // namespace qml
} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::qml::QmlLibraryTreeviewModel*)
