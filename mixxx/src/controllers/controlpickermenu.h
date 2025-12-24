#pragma once

#include <QMenu>
#include <QObject>

class ConfigKey;

class ControlPickerMenu : public QMenu {
    Q_OBJECT
  public:
    ControlPickerMenu(QWidget* pParent);
    virtual ~ControlPickerMenu();

    const QList<ConfigKey>& controlsAvailable() const {
        return m_controlsAvailable;
    }

    bool controlExists(const ConfigKey& key) const;
    QString descriptionForConfigKey(const ConfigKey& key) const;
    QString controlTitleForConfigKey(const ConfigKey& key) const;

    // share translated group strings
    QMap<QString, QString> getNumGroupsTrMap() const {
        return m_numGroupsTrMap;
    }
    QMap<QString, QString> getOtherGroupsTrMap() const {
        return m_otherGroupsTrMap;
    }

  signals:
    // Emitted when the user selects a control from the menu.
    void controlPicked(const ConfigKey& control);

  private slots:
    // Triggered when user selects a control from the menu.
    void controlChosen(int controlIndex);

  private:
    QMenu* addSubmenu(QString title, QMenu* pParent = NULL);
    void addSingleControl(const QString& group,
            const QString& control,
            const QString& title,
            const QString& description,
            QMenu* pMenu,
            const QString& prefix = QString(),
            const QString& actionTitle = QString());
    void addControl(const QString& group,
            const QString& control,
            const QString& title,
            const QString& helpText,
            QMenu* pMenu,
            bool addReset = false,
            const QString& prefix = QString());
    void addPlayerControl(const QString& control,
            const QString& title,
            const QString& helpText,
            QMenu* pMenu,
            bool deckControls,
#ifdef __STEM__
            bool deckStemControls,
#endif
            bool samplerControls,
            bool previewdeckControls,
            bool addReset = false);
    void addDeckAndSamplerControl(const QString& control,
            const QString& title,
            const QString& helpText,
            QMenu* pMenu,
#ifdef __STEM__
            bool stemsControls = false,
#endif
            bool addReset = false);
    void addDeckAndPreviewDeckControl(const QString& control,
            const QString& title,
            const QString& helpText,
            QMenu* pMenu,
#ifdef __STEM__
            bool stemsControls = false,
#endif
            bool addReset = false);
    void addDeckAndSamplerAndPreviewDeckControl(const QString& control,
            const QString& title,
            const QString& helpText,
            QMenu* pMenu,
#ifdef __STEM__
            bool stemsControls = false,
#endif
            bool addReset = false);
    void addDeckControl(const QString& control,
            const QString& title,
            const QString& helpText,
            QMenu* pMenu,
#ifdef __STEM__
            bool stemsControls = false,
#endif
            bool addReset = false);
    void addSamplerControl(const QString& control,
            const QString& title,
            const QString& helpText,
            QMenu* pMenu,
            bool addReset = false);
    void addPreviewDeckControl(const QString& control,
            const QString& title,
            const QString& helpText,
            QMenu* pMenu,
            bool addReset = false);
    void addMicrophoneAndAuxControl(const QString& control,
            const QString& title,
            const QString& helpText,
            QMenu* pMenu,
            bool microphoneControls,
            bool auxControls,
            bool addReset = false);
    void addLibraryControl(const QString& control,
            const QString& title,
            const QString& helpText,
            QMenu* pMenu);

    int addAvailableControl(const ConfigKey& key, const QString& title, const QString& description);

    QString m_effectMainOutputStr;
    QString m_effectHeadphoneOutputStr;
    QString m_deckStr;
    QString m_deckStemStr;
    QString m_previewdeckStr;
    QString m_samplerStr;
    QString m_resetStr;
    QString m_microphoneStr;
    QString m_auxStr;
    QString m_effectRackStr;
    QString m_effectUnitStr;
    QString m_effectStr;
    QString m_parameterStr;
    QString m_buttonParameterStr;
    QString m_libraryStr;

    QList<ConfigKey> m_controlsAvailable;
    QHash<ConfigKey, QString> m_descriptionsByKey;
    QHash<ConfigKey, QString> m_titlesByKey;

    QMap<QString, QString> m_numGroupsTrMap;
    QMap<QString, QString> m_otherGroupsTrMap;
};
