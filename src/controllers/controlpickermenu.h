#ifndef CONTROLPICKERMENU_H
#define CONTROLPICKERMENU_H

#include <QMenu>
#include <QObject>
#include <QSignalMapper>

#include "configobject.h"

class ControlPickerMenu : public QMenu {
    Q_OBJECT
  public:
    ControlPickerMenu(QWidget* pParent);
    virtual ~ControlPickerMenu();

    const QList<ConfigKey>& controlsAvailable() const {
        return m_controlsAvailable;
    }

    QString descriptionForConfigKey(ConfigKey key) const;
    QString prettyNameForConfigKey(ConfigKey key) const;

  signals:
    // Emitted when the user selects a control from the menu.
    void controlPicked(ConfigKey control);

  private slots:
    // Triggered when user selects a control from the menu.
    void controlChosen(int controlIndex);

  private:
    QMenu* addSubmenu(QString title, QMenu* pParent=NULL);
    void addControl(QString group, QString control, QString helpText,
                    QMenu* pMenu, bool addReset=false);
    void addPlayerControl(QString control, QString helpText, QMenu* pMenu,
                          bool deckControls, bool samplerControls,
                          bool previewdeckControls, bool addReset=false);
    void addDeckAndSamplerControl(QString control, QString helpText,
                                  QMenu* pMenu, bool addReset=false);
    void addDeckAndPreviewDeckControl(QString control, QString helpText,
                                      QMenu* pMenu, bool addReset=false);
    void addDeckAndSamplerAndPreviewDeckControl(QString control,
                                                QString helpText, QMenu* pMenu,
                                                bool addReset=false);
    void addDeckControl(QString control, QString helpText, QMenu* pMenu,
                        bool addReset=false);
    void addSamplerControl(QString control, QString helpText, QMenu* pMenu,
                           bool addReset=false);
    void addPreviewDeckControl(QString control, QString helpText, QMenu* pMenu,
                               bool addReset=false);
    void addMicrophoneAndAuxControl(QString control, QString helpText, QMenu* pMenu,
                                    bool microhoneControls, bool auxControls,
                                    bool addReset=false);
    void addEffectControl(QString group, QString control, QString menuDescription,
                          QString descriptionPrefix,
                          QMenu* pMenu, bool addReset=false);
    void addAvailableControl(ConfigKey key, QString description);

    QString m_masterOutputStr;
    QString m_headphoneOutputStr;
    QString m_deckStr;
    QString m_previewdeckStr;
    QString m_samplerStr;
    QString m_resetStr;
    QString m_microphoneStr;
    QString m_auxStr;
    QString m_effectRackStr;
    QString m_effectUnitStr;
    QString m_effectStr;
    QString m_parameterStr;

    QSignalMapper m_actionMapper;
    QList<ConfigKey> m_controlsAvailable;
    QHash<ConfigKey, QString> m_descriptionsByKey;
};

#endif /* CONTROLPICKERMENU_H */
