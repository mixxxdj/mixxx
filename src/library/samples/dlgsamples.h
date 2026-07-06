#pragma once

#include <QDrag>
#include <QLabel>
#include <QListWidget>
#include <QMimeData>
#include <QMouseEvent>
#include <QUrl>
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

// Custom QListWidget that supports drag-and-drop with proper text/uri-list MIME data.
// This enables dragging samples from the list to sampler boxes in QML.
class SampleListWidget : public QListWidget {
    Q_OBJECT
  public:
    SampleListWidget(QWidget* parent = nullptr)
            : QListWidget(parent) {
        setDragEnabled(true);
        setDragDropMode(QAbstractItemView::DragOnly);
        setDefaultDropAction(Qt::CopyAction);
        setMouseTracking(true);
        viewport()->setAcceptDrops(false);
    }

  protected:
    void startDrag(Qt::DropActions supportedActions) override {
        QListWidgetItem* pItem = currentItem();
        if (!pItem) {
            return;
        }

        QMimeData* mimeData = new QMimeData();
        QList<QUrl> urls;
        urls.append(QUrl::fromLocalFile(pItem->toolTip()));
        mimeData->setUrls(urls);
        // Also set text/uri-list explicitly for QML DropArea compatibility
        mimeData->setData(QStringLiteral("text/uri-list"),
                urls.first().toString(QUrl::FullyEncoded).toUtf8());

        QDrag* drag = new QDrag(this);
        drag->setMimeData(mimeData);

        // Create a simple pixmap from the item text
        QLabel label(pItem->text());
        label.setStyleSheet(QStringLiteral(
                "QLabel { background: palette(window); border: 1px solid palette(highlight);"
                " padding: 4px; }"));
        QPixmap pixmap = label.grab();
        drag->setPixmap(pixmap);

        drag->exec(Qt::CopyAction);
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        if (event->buttons() & Qt::LeftButton) {
            QListWidgetItem* pItem = itemAt(event->pos());
            if (pItem) {
                setCurrentItem(pItem);
            }
        }
        QListWidget::mouseMoveEvent(event);
    }
};

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
    void restoreSearch(const QString& search);

  private slots:
    void slotSampleActivated(QListWidgetItem* pItem);

  private:
    UserSettingsPointer m_pConfig;
    Library* m_pLibrary;
    SampleListWidget* m_pSampleList;
    QString m_currentSearch;
    QString m_samplesPath;
    QStringList m_sampleFiles;
};