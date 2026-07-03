#pragma once

#include <QListWidget>
#include <QWidget>

#include "library/libraryview.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"
#ifdef __STEM__
#include "engine/engine.h"
#endif

class WLibrary;
class Library;
class KeyboardEventFilter;

class DlgSamples : public QWidget, public virtual LibraryView {
    Q_OBJECT
  public:
    DlgSamples(WLibrary* parent,
            UserSettingsPointer pConfig,
            Library* pLibrary,
            KeyboardEventFilter* pKeyboard);
    ~DlgSamples() override;

    void onSearch(const QString& text) override;
    void onShow() override {
    }
    bool hasFocus() const override;
    void setFocus() override;
    inline const QString currentSearch() {
        return m_currentSearch;
    }
    void saveCurrentViewState() override;
    bool restoreCurrentViewState() override;

  public slots:
    void refreshBrowseModel();
    void slotRestoreSearch();

  signals:
    void loadTrackToPlayer(TrackPointer tio,
            const QString& group,
#ifdef __STEM__
            mixxx::StemChannelSelection stemMask,
#endif
            bool);

  private slots:
    void slotSampleActivated(QListWidgetItem* pItem);

  private:
    UserSettingsPointer m_pConfig;
    Library* m_pLibrary;
    QListWidget* m_pSampleList;
    QString m_currentSearch;
    QString m_samplesPath;
    QStringList m_sampleFiles;
};
