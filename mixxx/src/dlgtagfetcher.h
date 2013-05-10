#ifndef DLGTAGFETCHER_H
#define DLGTAGFETCHER_H

#include <QDialog>
#include "ui_dlgtagfetcher.h"
#include "trackinfoobject.h"


class QTreeWidget;


class DlgTagFetcher : public QDialog, public Ui::DlgTagFetcher {
  Q_OBJECT

  public:
    DlgTagFetcher(QWidget *parent);
    virtual ~DlgTagFetcher();

    void init(const TrackPointer track);

  signals:
    void next();
    void previous();
    void finished();
    void StartSubmit(TrackPointer);

  public slots:
    void FetchTagFinished(const TrackPointer,const QList<TrackPointer>& tracks);
    void ResultSelected();
    void FetchTagProgress(QString);

  private slots:
    void apply();
    void cancel();

  private:

    void UpdateStack();
    void AddDivider(const QString& text, QTreeWidget* parent) const;
    void AddTrack(const TrackPointer track, int resultIndex,
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
};

#endif // DLGTAGFETCHER_H
