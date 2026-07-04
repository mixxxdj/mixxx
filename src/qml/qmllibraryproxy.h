#pragma once
#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <Qt>
#include <memory>

#include "qml_owned_ptr.h"
#include "qmllibrarytracklistmodel.h"

class Library;
class KeyboardEventFilter;
class WTrackMenu;

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
    ~QmlLibraryProxy() override;

    static QmlLibraryProxy* create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine);
    static void registerLibrary(std::shared_ptr<Library> pLibrary) {
        s_pLibrary = std::move(pLibrary);
    }

    static Library* get() {
        return s_pLibrary.get();
    }

    static void registerKeyboardEventFilter(std::shared_ptr<KeyboardEventFilter> pKeyboard) {
        s_pKeyboard = std::move(pKeyboard);
    }

    static KeyboardEventFilter* getKeyboard() {
        return s_pKeyboard.get();
    }

    QmlLibraryTrackListModel* model() const;
    Q_INVOKABLE void analyze(const mixxx::qml::QmlTrackProxy* track) const;
    Q_INVOKABLE void showDeckTrackMenu(
            mixxx::qml::QmlTrackProxy* track,
            const QString& group,
            const QString& property,
            int globalXPosition,
            int globalYPosition);
    Q_INVOKABLE void showDeckTrackProperties(
            mixxx::qml::QmlTrackProxy* track,
            const QString& group,
            const QString& property);
    Q_INVOKABLE QString deckHotcueLabel(
            mixxx::qml::QmlTrackProxy* track,
            int hotcueNumber) const;
    Q_INVOKABLE bool setDeckHotcueLabel(
            mixxx::qml::QmlTrackProxy* track,
            int hotcueNumber,
            const QString& label);
    Q_INVOKABLE bool setDeckHotcueType(
            mixxx::qml::QmlTrackProxy* track,
            const QString& group,
            int hotcueNumber,
            const QString& action);
    Q_INVOKABLE void cleanupDeckHotcuePopup(
            mixxx::qml::QmlTrackProxy* track,
            int hotcueNumber);

  private:
    void ensureDeckTrackMenu();

    static inline std::shared_ptr<Library> s_pLibrary;
    static inline std::shared_ptr<KeyboardEventFilter> s_pKeyboard;

    std::unique_ptr<WTrackMenu> m_pDeckTrackMenu;
};

} // namespace qml
} // namespace mixxx
