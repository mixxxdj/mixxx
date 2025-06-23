#pragma once

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QRadioButton>

#include "preferences/usersettings.h"
#include "util/optional.h"
#include "util/parented_ptr.h"

class TrackCollectionManager;
class QWidget;
class CrateId;

namespace mixxx {

struct EnginePrimeExportRequest;

/// The DlgLibraryExport class is a UI window that gathers information from
/// the user about how they would like to export the Mixxx library.
///
/// Currently, the dialog only supports exporting to the Engine Library format,
/// but in future it is expected that this dialog could be expanded to include
/// other formats, and generate different export signals accordingly.
class DlgLibraryExport : public QDialog {
    Q_OBJECT

  public:
    DlgLibraryExport(
            QWidget* parent,
            UserSettingsPointer pConfig,
            TrackCollectionManager* pTrackCollectionManager);

    /// Set the specified crate to be selected for export on the dialog.  If no
    /// crate is provided (i.e. `std::nullopt`), then the dialog will be ready
    /// to export the whole library.  If an unknown crate is provided, then no
    /// action is taken.
    void setSelectedCrate(std::optional<CrateId> crateId);

    /// Refresh the contents of the dialog.
    void refresh();

  signals:
    /// The startEnginePrimeExport signal is emitted when sufficient information
    /// has been gathered from the user to kick off an Engine DJ export, and
    /// details of the request are provided as part of the signal.
    void startEnginePrimeExport(QSharedPointer<mixxx::EnginePrimeExportRequest>);

  private slots:
    void browseExportDirectory();
    void exportRequested();

  private:
    void checkExistingDatabase();

    UserSettingsPointer m_pConfig;
    TrackCollectionManager* m_pTrackCollectionManager;

    parented_ptr<QRadioButton> m_pWholeLibraryRadio;
    parented_ptr<QRadioButton> m_pCratesRadio;
    parented_ptr<QListWidget> m_pCratesList;
    parented_ptr<QLineEdit> m_pExportDirectoryTextField;
    parented_ptr<QComboBox> m_pVersionCombo;
    parented_ptr<QLabel> m_pExistingDatabaseLabel;
};

} // namespace mixxx
