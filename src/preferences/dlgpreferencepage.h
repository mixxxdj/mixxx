#ifndef DLGPREFERENCEPAGE_H
#define DLGPREFERENCEPAGE_H

#include <QWidget>

// API that all preference pages should implement.
class DlgPreferencePage : public QWidget {
    Q_OBJECT
  public:
    DlgPreferencePage(QWidget* pParent);
    virtual ~DlgPreferencePage();

  public slots:
    // Called when the preference dialog is shown to the user (not necessarily
    // when this PreferencePage is shown to the user). At this point, the
    // PreferencePage should update all of its setting to the latest values.
    virtual void slotUpdate() {}

    // Called when the user clicks the global "Apply" button. The preference
    // dialog should make all of the current setting of the UI widgets active.
    virtual void slotApply() {}

    // Called when the user clicks the global "Cancel" button. The preference
    // dialog should revert all of the changes the user made since the last
    // slotUpdate.
    virtual void slotCancel() {}

    // Called when the user clicks the global "Reset to Defaults" button. The
    // preference dialog should revert settings to their default values.
    virtual void slotResetToDefaults() {}

    // Called when the preferences dialog is shown to the user (not necessarily
    // when this PreferencePage is shown to the user).
    virtual void slotShow() {}

    // Called when the preferences dialog is hidden from the user.
    virtual void slotHide() {}
};


#endif /* DLGPREFERENCEPAGE_H */
