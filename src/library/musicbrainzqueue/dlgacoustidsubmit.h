#pragma once

#include <QDialog>

#include "library/musicbrainzqueue/ui_dlgacoustidsubmit.h"
#include "preferences/usersettings.h"

class DlgAcoustIdSubmit : public QDialog, public Ui::DlgAcoustIdSubmit {
    Q_OBJECT

  public:
    /// @param parent       parent widget (the MBQueue view)
    /// @param pConfig      user settings — read for existing key, written if Remember is checked
    /// @param trackCount   number of tracks about to be submitted; shown in the label
    explicit DlgAcoustIdSubmit(
            QWidget* parent,
            UserSettingsPointer pConfig,
            int trackCount);
    ~DlgAcoustIdSubmit() override = default;

    /// Returns the API key currently in the line edit (trimmed).
    /// Valid to call after exec() returns QDialog::Accepted.
    QString apiKey() const;

    /// Returns true if the user checked "Remember key and auto-submit".
    bool rememberKey() const;

  private slots:
    /// Called when the text in the key field changes.
    /// Enables or disables the Submit button to prevent submission with an empty key.
    void slotApiKeyTextChanged(const QString& text);

  private:
    UserSettingsPointer m_pConfig;
};
