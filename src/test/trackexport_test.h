#include "library/export/trackexportworker.h"

#include <future>

#include <gtest/gtest.h>

#include <QDateTime>
#include <QDebug>
#include <QPair>
#include <QTemporaryDir>

#include "track/track.h"


class FakeOverwriteAnswerer : public QObject {
    Q_OBJECT
  public:
    FakeOverwriteAnswerer(TrackExportWorker* worker) : m_worker(worker) {
        connect(m_worker, SIGNAL(progress(QString, int, int)), this,
            SLOT(slotProgress(QString, int, int)));
        connect(m_worker,
            SIGNAL(askOverwriteMode(
                    QString, std::promise<TrackExportWorker::OverwriteAnswer>*)),
            this,
            SLOT(slotAskOverwriteMode(
                    QString,
                    std::promise<TrackExportWorker::OverwriteAnswer>*)));
        connect(m_worker, SIGNAL(canceled()), this, SLOT(cancelButtonClicked()));
    }
    virtual ~FakeOverwriteAnswerer();

    void setAnswer(QString expected_filename,
                   TrackExportWorker::OverwriteAnswer answer) {
        // We should never copy a duplicate filename, so if a name already
        // exists that's a bug in the test.
        Q_ASSERT(m_answers.find(expected_filename) == m_answers.end());
        m_answers[expected_filename] = answer;
    }

    QString currentProgressFilename() const {
        return m_progress_filename;
    }

    int currentProgress() const {
        return m_progress;
    }

    int currentProgressCount() const {
        return m_progress_count;
    }

  public slots:
    void slotProgress(QString filename, int progress, int count);
    void slotAskOverwriteMode(
            QString filename,
            std::promise<TrackExportWorker::OverwriteAnswer>* promise);
    void cancelButtonClicked();

  private:
    TrackExportWorker* m_worker;
    QMap<QString, TrackExportWorker::OverwriteAnswer> m_answers;
    QString m_progress_filename;
    int m_progress = 0;
    int m_progress_count = 0;
};

class TrackExporterTest : public testing::Test {
  public:
    TrackExporterTest() :
        m_testDataDir(QDir::current().absoluteFilePath(
                "src/test/id3-test-data")) { }

    void SetUp() override {
        ASSERT_TRUE(m_exportTempDir.isValid());
        m_exportDir = QDir(m_exportTempDir.path());
    }

  protected:
    const QDir m_testDataDir;
    QTemporaryDir m_exportTempDir;
    QDir m_exportDir;
    QScopedPointer<FakeOverwriteAnswerer> m_answerer;
};
