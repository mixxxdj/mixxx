#pragma once
#include <QObject>
#include <memory>

#include "skin/qml/qmllibrarytracklistmodel.h"
#include "util/parented_ptr.h"

class Library;

namespace mixxx {
namespace skin {
namespace qml {

class QmlLibraryTrackListModel;

class QmlLibraryProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(mixxx::skin::qml::QmlLibraryTrackListModel* model MEMBER m_pModel CONSTANT)

  public:
    explicit QmlLibraryProxy(std::shared_ptr<Library> pLibrary, QObject* parent = nullptr);

  private:
    std::shared_ptr<Library> m_pLibrary;
    parented_ptr<QmlLibraryTrackListModel> m_pModel;
};

} // namespace qml
} // namespace skin
} // namespace mixxx
