#pragma once

#include <QAction>

#include "preferences/usersettings.h"

class Track;

class FindOnWebLast : public QAction {
    Q_OBJECT
  public:
    explicit FindOnWebLast(QWidget* pParent, UserSettingsPointer pConfig);
    ~FindOnWebLast() override = default;

    void update(
            const QString& actionId,
            const QString& actionText,
            const QUrl& serviceUrl);

    void init(
            const QString& actionId,
            const QString& actionText,
            const QUrl& serviceUrl);

  private:
    void setAction(
            const QString& actionId,
            const QString& actionText,
            const QUrl& serviceUrl);
    void openInBrowser(const QUrl& url);
    UserSettingsPointer m_pConfig;
    QString m_lastActionKey;
};
