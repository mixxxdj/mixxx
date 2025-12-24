#pragma once
#include <QObject>
#include <QQmlEngine>
#include <Qt>
#include <memory>

#include "qml_owned_ptr.h"
#include "qmllibrarytracklistmodel.h"

class Library;

namespace mixxx {
namespace qml {

class QmlTrackProxy;

class QmlLibraryProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(mixxx::qml::QmlLibraryTrackListModel* model READ model CONSTANT)
    QML_NAMED_ELEMENT(Library)
    QML_SINGLETON

  public:
    explicit QmlLibraryProxy(QObject* parent = nullptr);

    static QmlLibraryProxy* create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine);
    static void registerLibrary(std::shared_ptr<Library> pLibrary) {
        s_pLibrary = std::move(pLibrary);
    }

    static Library* get() {
        return s_pLibrary.get();
    }

    QmlLibraryTrackListModel* model() const;
    Q_INVOKABLE void analyze(const mixxx::qml::QmlTrackProxy* track) const;

  private:
    static inline std::shared_ptr<Library> s_pLibrary;
};

} // namespace qml
} // namespace mixxx
