#include "findonweblast.h"

#include <QtDebug>

#include "moc_findonweblast.cpp"
#include "track/track.h"
#include "util/desktophelper.h"

namespace {
const QString kLibraryGroup = QStringLiteral("[Library]");
const QString kFindOnWebLastActionKey = QStringLiteral("find_on_web_last_action");
} // namespace

FindOnWebLast::FindOnWebLast(QWidget* pParent, UserSettingsPointer pConfig)
        : QAction(pParent),
          m_pConfig(std::move(pConfig)) {
    m_lastActionKey = m_pConfig->getValueString(
            ConfigKey(kLibraryGroup, kFindOnWebLastActionKey));
}

void FindOnWebLast::openInBrowser(const QUrl& url) {
    if (!mixxx::DesktopHelper::openUrl(url)) {
        qWarning() << "DesktopHelper::openUrl() failed for " << url;
        DEBUG_ASSERT(!"openInBrowser() failed");
    }
}

void FindOnWebLast::update(
        const QString& actionId,
        const QString& actionText,
        const QUrl& serviceUrl) {
    if (m_lastActionKey != actionId) {
        setAction(actionId, actionText, serviceUrl);
        m_pConfig->setValue(
                ConfigKey(kLibraryGroup, kFindOnWebLastActionKey), actionId);
        m_lastActionKey = actionId;
    }
}

void FindOnWebLast::init(
        const QString& actionId,
        const QString& actionText,
        const QUrl& serviceUrl) {
    if (m_lastActionKey == actionId) {
        setAction(actionId, actionText, serviceUrl);
    }
}

void FindOnWebLast::setAction(
        const QString& actionId,
        const QString& actionText,
        const QUrl& serviceUrl) {
    int firstCommaPos = actionId.indexOf(',');
    VERIFY_OR_DEBUG_ASSERT(firstCommaPos >= 0) {
        return;
    }

    QStringView service = QStringView(actionId).left(firstCommaPos);
    //: Menu entry like "Find Artist on Wikipedia"
    setText(tr("Find %1 on %2").arg(actionText, service));
    disconnect();
    connect(this, &QAction::triggered, this, [this, serviceUrl] {
        openInBrowser(serviceUrl);
    });
    setVisible(true);
}
