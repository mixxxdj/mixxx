#pragma once
#include <QObject>
#include <QString>

class Library;
class SidebarModel;
class QAbstractItemModel;

namespace mixxx {
namespace skin {
namespace qml {

class QmlLibraryProxy : public QObject {
    Q_OBJECT
  public:
    explicit QmlLibraryProxy(Library* pLibrary, QObject* parent = nullptr);

    Q_INVOKABLE QObject* getSidebarModel();
    Q_INVOKABLE QObject* getLibraryModel();

  private:
    Library* m_pLibrary;
};

} // namespace qml
} // namespace skin
} // namespace mixxx
