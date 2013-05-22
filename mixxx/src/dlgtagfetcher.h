#ifndef DLGTAGFETCHER_H
#define DLGTAGFETCHER_H

#include <QWidget>
#include "ui_dlgtagfetcher.h"
#include "trackinfoobject.h"
#include "musicbrainz/tagfetcher.h"


class QTreeWidget;


class DlgTagFetcher : public QWidget,  public Ui::DlgTagFetcher {
  Q_OBJECT

  public:
    DlgTagFetcher(QWidget *parent);
    virtual ~DlgTagFetcher();

    void init(const TrackPointer track);

  signals:
    void next();
    void previous();

  public slots:
    void fetchTagFinished(const TrackPointer,const QList<TrackPointer>& tracks);
    void resultSelected();
    void fetchTagProgress(QString);

  private slots:
    void apply();
    void quit();

  private:

    void updateStack();
    void addDivider(const QString& text, QTreeWidget* parent) const;
    void addTrack(const TrackPointer track, int resultIndex,
                  QTreeWidget* parent) const;

  private:
    struct Data {
        Data() : m_pending(true), m_selectedResult(0) {}

        bool m_pending;
        int m_selectedResult;
        QList<TrackPointer> m_results;
    };

    TrackPointer m_track;
    Data m_data;
    QString m_progress;
    TagFetcher m_TagFetcher;
};

#endif // DLGTAGFETCHER_H
