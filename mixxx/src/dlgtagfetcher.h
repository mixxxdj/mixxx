#ifndef DLGTAGFETCHER_H
#define DLGTAGFETCHER_H

#include <QDialog>
#include "ui_dlgtagfetcher.h"
#include "trackinfoobject.h"
#include "configobject.h"


class QTreeWidget;


class DlgTagFetcher : public QDialog, public Ui::DlgTagFetcher {
  Q_OBJECT

  public:
    DlgTagFetcher(QWidget *parent, ConfigObject<ConfigValue>* pConfig);
    virtual ~DlgTagFetcher();

    void init(const TrackPointer track);

  signals:
    void next();
    void previous();
    void finished();
    void StartSubmit(TrackPointer, QString);

  public slots:
    void FetchTagFinished(const TrackPointer,const QList<TrackPointer>& tracks);
    void submitProgress(QString);
    void ResultSelected();
    void submitFinished(int,QString);
    void FetchTagProgress(QString);

  private slots:
    void apply();
    void cancel();
    void submitPage();
    void submit();
    void getApiKey();

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
    bool m_submit;
    QString m_progress;
    ConfigObject<ConfigValue>* m_pConfig;
};

#endif // DLGTAGFETCHER_H


