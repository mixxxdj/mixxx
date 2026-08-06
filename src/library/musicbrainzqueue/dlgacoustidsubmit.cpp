#include "library/musicbrainzqueue/dlgacoustidsubmit.h"

#include <QPushButton>

#include "library/library_prefs.h"
#include "moc_dlgacoustidsubmit.cpp"
#include "util/desktophelper.h"

using namespace mixxx::library::prefs;

namespace {

const QString kAcoustIdApiKeyUrl =
        QStringLiteral("https://acoustid.org/api-key");

} // anonymous namespace

DlgAcoustIdSubmit::DlgAcoustIdSubmit(
        QWidget* parent,
        UserSettingsPointer pConfig,
        int trackCount)
        : QDialog(parent),
          m_pConfig(pConfig) {
    setupUi(this);

    // Rename the OK button to "Submit" so the action is unambiguous.
    QPushButton* pSubmitButton = buttonBox->button(QDialogButtonBox::Ok);
    VERIFY_OR_DEBUG_ASSERT(pSubmitButton) {
        return;
    }
    pSubmitButton->setText(tr("Submit"));

    // Populate the track count label.
    labelTrackCount->setText(tr("Submitting %n track(s).", "", trackCount));

    labelGetKey->setText(
            QStringLiteral("<a href=\"%1\">%2</a>")
                    .arg(kAcoustIdApiKeyUrl, tr("Get ↗")));
    connect(labelGetKey,
            &QLabel::linkActivated,
            [](const QString& url) {
                mixxx::DesktopHelper::openUrl(url);
            });

    // Pre-fill the key field if one is already saved in Preferences,
    // so the user does not have to retype it every time auto-submit is off.
    const QString savedKey = m_pConfig->getValue(
            kAcoustIdUserApiKeyConfigKey, QString());
    lineEditApiKey->setText(savedKey);

    // Disable Submit until the key field contains something.
    // slotApiKeyTextChanged also fires immediately via setText above.
    connect(lineEditApiKey,
            &QLineEdit::textChanged,
            this,
            &DlgAcoustIdSubmit::slotApiKeyTextChanged);
    slotApiKeyTextChanged(savedKey);

    // The dialog is modal — make it appear centred over the parent.
    setModal(true);
}

QString DlgAcoustIdSubmit::apiKey() const {
    return lineEditApiKey->text().trimmed();
}

bool DlgAcoustIdSubmit::rememberKey() const {
    return checkBoxRemember->isChecked();
}

void DlgAcoustIdSubmit::slotApiKeyTextChanged(const QString& text) {
    QPushButton* pSubmitButton = buttonBox->button(QDialogButtonBox::Ok);
    if (pSubmitButton) {
        pSubmitButton->setEnabled(!text.trimmed().isEmpty());
    }
}
