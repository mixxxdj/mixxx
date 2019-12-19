#pragma once

#include <QDialog>
#include <QList>
#include <QTreeWidget>

#include "library/ui_dlgtagfetcher.h"
#include "track/track.h"
#include "musicbrainz/tagfetcher.h"

class DlgTagFetcher : public QDialog,  public Ui::DlgTagFetcher {
  Q_OBJECT

  public:
    explicit DlgTagFetcher(QWidget *parent);
    ~DlgTagFetcher() override = default;

    void init();

  public slots:
    void loadTrack(const TrackPointer& track);
    void updateTrackMetadata(Track* pTIO);

  signals:
    void next();
    void previous();

  private slots:
    void fetchTagFinished(const TrackPointer,const QList<TrackPointer>& tracks);
    void resultSelected();
    void fetchTagProgress(QString);
    void slotNetworkResult(int httpStatus, QString app, QString message, int code);
    void apply();
    void quit();

  private:
    void updateStack();
    void addDivider(const QString& text, QTreeWidget* parent) const;
    void addTrack(const TrackPointer track, int resultIndex,
                  QTreeWidget* parent) const;

    TagFetcher m_tagFetcher;

    TrackPointer m_track;

    struct Data {
        Data() : m_pending(true), m_selectedResult(-1) {}

        bool m_pending;
        int m_selectedResult;
        QList<TrackPointer> m_results;
    };
    Data m_data;

    enum class NetworkResult {
        Ok,
        HttpError,
        UnknownError,
    };
    NetworkResult m_networkResult;
};
