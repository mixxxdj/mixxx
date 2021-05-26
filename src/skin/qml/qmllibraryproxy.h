#pragma once
#include <QObject>

class Library;

namespace mixxx {
namespace skin {
namespace qml {

class QmlLibraryProxy : public QObject {
    Q_OBJECT
  public:
    explicit QmlLibraryProxy(Library* pLibrary, QObject* parent = nullptr);

    Q_INVOKABLE QAbstractItemModel* model();

  private:
    Library* m_pLibrary;
};

} // namespace qml
} // namespace skin
} // namespace mixxx
