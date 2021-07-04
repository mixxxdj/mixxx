#pragma once
#include <QObject>

class Library;

namespace mixxx {
namespace skin {
namespace qml {

class QmlLibraryTrackListModel;

class QmlLibraryProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(mixxx::skin::qml::QmlLibraryTrackListModel* model MEMBER m_pModel CONSTANT)

  public:
    explicit QmlLibraryProxy(Library* pLibrary, QObject* parent = nullptr);
    ~QmlLibraryProxy();

  private:
    Library* m_pLibrary;
    QmlLibraryTrackListModel* m_pModel;
};

} // namespace qml
} // namespace skin
} // namespace mixxx
