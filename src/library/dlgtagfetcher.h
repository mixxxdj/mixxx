#ifndef DLGTAGFETCHER_H
#define DLGTAGFETCHER_H

#include <QDialog>
#include <QTreeWidget>

#include "library/ui_dlgtagfetcher.h"
#include "track/track.h"
#include "musicbrainz/tagfetcher.h"

class DlgTagFetcher : public QDialog,  public Ui::DlgTagFetcher {
  Q_OBJECT

  public:
    DlgTagFetcher(QWidget *parent);
    virtual ~DlgTagFetcher();

    void init();

    enum networkError {
        NOERROR,
        HTTPERROR,
        FTWERROR
    };

  public slots:
    void loadTrack(const TrackPointer track);
    void updateTrackMetadata(Track* pTIO);

  signals:
    void next();
    void previous();

  private slots:
    void fetchTagFinished(const TrackPointer,const QList<TrackPointer>& tracks);
    void resultSelected();
    void fetchTagProgress(QString);
    void slotNetworkError(int httpStatus, QString app, QString message, int code);
    void apply();
    void quit();

  private:
    void updateStack();
    void addDivider(const QString& text, QTreeWidget* parent) const;
    void addTrack(const TrackPointer track, int resultIndex,
                  QTreeWidget* parent) const;
    struct Data {
        Data() : m_pending(true), m_selectedResult(-1) {}

        bool m_pending;
        int m_selectedResult;
        QList<TrackPointer> m_results;
    };

    TrackPointer m_track;
    Data m_data;
    QString m_progress;
    TagFetcher m_TagFetcher;
    networkError m_networkError;
};

#endif // DLGTAGFETCHER_H
