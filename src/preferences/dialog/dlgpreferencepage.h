#pragma once

#include <QString>
#include <QUrl>
#include <QWidget>

#include "util/color/color.h"
#include "util/string.h"

/// Interface that all preference pages have to implement.
class DlgPreferencePage : public QWidget {
    Q_OBJECT
  public:
    DlgPreferencePage(QWidget* pParent);
    virtual ~DlgPreferencePage();

    /// Returns the help URL for the current page.
    /// Subclasses can provide a path to the appropriate manual page by
    /// overriding this. The default implementation returns an invalid QUrl.
    virtual QUrl helpUrl() const;

    QColor m_pLinkColor;

  public slots:
    /// Called when the preference dialog is shown to the user (not necessarily
    /// when this PreferencePage is shown to the user). At this point, the
    /// PreferencePage should update all of its setting to the latest values.
    virtual void slotUpdate() = 0;

    /// Called when the user clicks the global "Apply" button. The preference
    /// dialog should make all of the current setting of the UI widgets active.
    virtual void slotApply() = 0;

    /// Called when the user clicks the global "Cancel" button. The preference
    /// dialog should revert all of the changes the user made since the last
    /// slotUpdate. The default implementation just class slotUpdate.
    virtual void slotCancel() {
        slotUpdate();
    }

    /// Called when the user clicks the global "Reset to Defaults" button. The
    /// preference dialog should revert settings to their default values.
    virtual void slotResetToDefaults() = 0;

    /// Called when the preferences dialog is shown to the user (not necessarily
    /// when this PreferencePage is shown to the user).
    virtual void slotShow() {
    }

    /// Called when the preferences dialog is hidden from the user.
    virtual void slotHide() {
    }

    // Supply a readable text color for all file and website links
    inline void createLinkColor() {
        // Blend the palette colors for regular text and link text to get a color
        // that is more likely to be visible with dark OS themes.
        // https://bugs.launchpad.net/mixxx/+bug/1900201
        m_pLinkColor = Color::blendColors(palette().link().color(),
                palette().text().color())
                               .name();
    }
};
