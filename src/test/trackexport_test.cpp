// Exercises the track export worker.
// Just uses temp directories rather than trying to mock the filesystem.

#include "test/trackexport_test.h"

#include <QDebug>
#include <QScopedPointer>

#include "moc_trackexport_test.cpp"
#include "track/track.h"

FakeOverwriteAnswerer::~FakeOverwriteAnswerer() { }

void FakeOverwriteAnswerer::slotProgress(const QString& filename, int progress, int count) {
    m_progress_filename = filename;
    m_progress = progress;
    m_progress_count = count;
}

void FakeOverwriteAnswerer::slotAskOverwriteMode(
        const QString& filename,
        std::promise<TrackExportWorker::OverwriteAnswer>* promise) {
    auto it = m_answers.find(filename);
    // Make sure this filename is in the map, and then remove it.
    // The ASSERT_EQ macro has trouble with this comparison.
    ASSERT_FALSE(it == m_answers.end());
    TrackExportWorker::OverwriteAnswer answer = m_answers[filename];
    m_answers.remove(filename);

    qDebug() << "Asked about" << filename << "Answering: "
            << static_cast<int>(answer);
    promise->set_value(answer);
}

void FakeOverwriteAnswerer::cancelButtonClicked() {
    m_progress = -1;
    m_progress_count = -1;
}

namespace {
const QString kOggTestFile = QStringLiteral("cover-test-øé~ł€˚.ogg");
const QString kFlacTestFile = QStringLiteral("cover-test-øé~ł€˚.flac");
const QString kM4aAacTestFile = QStringLiteral("cover-test-øé~ł€˚-ffmpeg-aac.m4a");
} // namespace

TEST_F(TrackExporterTest, SimpleListExport) {
    // Create a simple list of trackpointers and export them.
    mixxx::FileInfo fileinfo1(m_testDataDir.filePath(kOggTestFile));
    TrackPointer track1(Track::newTemporary(mixxx::FileAccess(fileinfo1)));
    mixxx::FileInfo fileinfo2(m_testDataDir.filePath(kFlacTestFile));
    TrackPointer track2(Track::newTemporary(mixxx::FileAccess(fileinfo2)));
    mixxx::FileInfo fileinfo3(m_testDataDir.filePath(kM4aAacTestFile));
    TrackPointer track3(Track::newTemporary(mixxx::FileAccess(fileinfo3)));

    // An initializer list would be prettier here, but it doesn't compile
    // on MSVC or OSX.
    TrackPointerList tracks;
    tracks.append(track1);
    tracks.append(track2);
    tracks.append(track3);
    TrackExportWorker worker(m_exportDir.canonicalPath(), tracks);
    m_answerer.reset(new FakeOverwriteAnswerer(&worker));

    worker.run();
    EXPECT_TRUE(worker.wait(10000));

    EXPECT_EQ(3, m_answerer->currentProgress());
    EXPECT_EQ(3, m_answerer->currentProgressCount());

    // The destination folder should have all the files.
    EXPECT_TRUE(QFileInfo::exists(m_exportDir.filePath(kOggTestFile)));
    EXPECT_TRUE(QFileInfo::exists(m_exportDir.filePath(kFlacTestFile)));
    EXPECT_TRUE(QFileInfo::exists(m_exportDir.filePath(kM4aAacTestFile)));
}

TEST_F(TrackExporterTest, OverwriteSkip) {
    // Export a tracklist with two existing tracks -- overwrite one and skip
    // the other.
    mixxx::FileInfo fileinfo1(m_testDataDir.filePath(kOggTestFile));
    const qint64 fileSize1 = fileinfo1.sizeInBytes();
    TrackPointer track1(Track::newTemporary(mixxx::FileAccess(fileinfo1)));
    mixxx::FileInfo fileinfo2(m_testDataDir.filePath(kM4aAacTestFile));
    TrackPointer track2(Track::newTemporary(mixxx::FileAccess(fileinfo2)));

    // Create empty versions at the destination so we can see if we actually
    // overwrote or skipped.
    QFile file1(m_exportDir.filePath(kOggTestFile));
    ASSERT_TRUE(file1.open(QIODevice::WriteOnly));
    file1.close();
    QFile file2(m_exportDir.filePath(kM4aAacTestFile));
    ASSERT_TRUE(file2.open(QIODevice::WriteOnly));
    file2.close();

    // Set up the worker and answerer.
    TrackPointerList tracks;
    tracks.append(track1);
    tracks.append(track2);
    TrackExportWorker worker(m_exportDir.canonicalPath(), tracks);
    m_answerer.reset(new FakeOverwriteAnswerer(&worker));
    m_answerer->setAnswer(QFileInfo(file1).canonicalFilePath(),
                           TrackExportWorker::OverwriteAnswer::OVERWRITE);
    m_answerer->setAnswer(QFileInfo(file2).canonicalFilePath(),
                           TrackExportWorker::OverwriteAnswer::SKIP);

    worker.run();
    EXPECT_TRUE(worker.wait(10000));

    EXPECT_EQ(2, m_answerer->currentProgress());
    EXPECT_EQ(2, m_answerer->currentProgressCount());

    // The destination folder should have both the files, one skipped and
    // one written.
    QFileInfo newfile1(m_exportDir.filePath(kOggTestFile));
    EXPECT_TRUE(newfile1.exists());
    EXPECT_EQ(fileSize1, newfile1.size());

    QFileInfo newfile2(m_exportDir.filePath(kM4aAacTestFile));
    EXPECT_TRUE(newfile2.exists());
    EXPECT_EQ(0, newfile2.size());
}

TEST_F(TrackExporterTest, OverwriteAll) {
    // Export a tracklist with two existing tracks -- overwrite both.
    mixxx::FileInfo fileinfo1(m_testDataDir.filePath(kOggTestFile));
    const qint64 fileSize1 = fileinfo1.sizeInBytes();
    TrackPointer track1(Track::newTemporary(mixxx::FileAccess(fileinfo1)));
    mixxx::FileInfo fileinfo2(m_testDataDir.filePath(kM4aAacTestFile));
    const qint64 fileSize2 = fileinfo2.sizeInBytes();
    TrackPointer track2(Track::newTemporary(mixxx::FileAccess(fileinfo2)));

    // Create empty versions at the destination so we can see if we actually
    // overwrote or skipped.
    QFile file1(m_exportDir.filePath(kOggTestFile));
    ASSERT_TRUE(file1.open(QIODevice::WriteOnly));
    file1.close();
    QFile file2(m_exportDir.filePath(kM4aAacTestFile));
    ASSERT_TRUE(file2.open(QIODevice::WriteOnly));
    file2.close();

    // Set up the worker and answerer.
    TrackPointerList tracks;
    tracks.append(track1);
    tracks.append(track2);
    TrackExportWorker worker(m_exportDir.canonicalPath(), tracks);
    m_answerer.reset(new FakeOverwriteAnswerer(&worker));
    m_answerer->setAnswer(QFileInfo(file2).canonicalFilePath(),
                           TrackExportWorker::OverwriteAnswer::OVERWRITE_ALL);

    worker.run();
    EXPECT_TRUE(worker.wait(10000));

    EXPECT_EQ(2, m_answerer->currentProgress());
    EXPECT_EQ(2, m_answerer->currentProgressCount());

    // Both files should have been overwritten.
    QFileInfo newfile1(m_exportDir.filePath(kOggTestFile));
    EXPECT_TRUE(newfile1.exists());
    EXPECT_EQ(fileSize1, newfile1.size());

    QFileInfo newfile2(m_exportDir.filePath(kM4aAacTestFile));
    EXPECT_TRUE(newfile2.exists());
    EXPECT_EQ(fileSize2, newfile2.size());
}

TEST_F(TrackExporterTest, SkipAll) {
    // Export a tracklist with two existing tracks -- skip both.
    mixxx::FileInfo fileinfo1(m_testDataDir.filePath(kOggTestFile));
    TrackPointer track1(Track::newTemporary(mixxx::FileAccess(fileinfo1)));
    mixxx::FileInfo fileinfo2(m_testDataDir.filePath(kM4aAacTestFile));
    TrackPointer track2(Track::newTemporary(mixxx::FileAccess(fileinfo2)));

    // Create empty versions at the destination so we can see if we actually
    // overwrote or skipped.
    QFile file1(m_exportDir.filePath(kOggTestFile));
    ASSERT_TRUE(file1.open(QIODevice::WriteOnly));
    file1.close();
    QFile file2(m_exportDir.filePath(kM4aAacTestFile));
    ASSERT_TRUE(file2.open(QIODevice::WriteOnly));
    file2.close();

    // Set up the worker and answerer.
    TrackPointerList tracks;
    tracks.append(track1);
    tracks.append(track2);
    TrackExportWorker worker(m_exportDir.canonicalPath(), tracks);
    m_answerer.reset(new FakeOverwriteAnswerer(&worker));
    m_answerer->setAnswer(QFileInfo(file2).canonicalFilePath(),
                           TrackExportWorker::OverwriteAnswer::SKIP_ALL);

    worker.run();
    EXPECT_TRUE(worker.wait(10000));

    EXPECT_EQ(2, m_answerer->currentProgress());
    EXPECT_EQ(2, m_answerer->currentProgressCount());

    // Both files should have been skipped.
    QFileInfo newfile1(m_exportDir.filePath(kOggTestFile));
    EXPECT_TRUE(newfile1.exists());
    EXPECT_EQ(0, newfile1.size());

    QFileInfo newfile2(m_exportDir.filePath(kM4aAacTestFile));
    EXPECT_TRUE(newfile2.exists());
    EXPECT_EQ(0, newfile2.size());
}

TEST_F(TrackExporterTest, Cancel) {
    // Export a tracklist with two existing tracks, but cancel before we do
    // anything.
    mixxx::FileInfo fileinfo1(m_testDataDir.filePath(kOggTestFile));
    TrackPointer track1(Track::newTemporary(mixxx::FileAccess(fileinfo1)));
    mixxx::FileInfo fileinfo2(m_testDataDir.filePath(kM4aAacTestFile));
    TrackPointer track2(Track::newTemporary(mixxx::FileAccess(fileinfo2)));

    // Create empty version at the destination so we can see if we actually
    // canceled.
    QFile file2(m_exportDir.filePath(kM4aAacTestFile));
    ASSERT_TRUE(file2.open(QIODevice::WriteOnly));
    file2.close();

    // Set up the worker and answerer.
    TrackPointerList tracks;
    tracks.append(track1);
    tracks.append(track2);
    TrackExportWorker worker(m_exportDir.canonicalPath(), tracks);
    m_answerer.reset(new FakeOverwriteAnswerer(&worker));
    m_answerer->setAnswer(QFileInfo(file2).canonicalFilePath(),
                           TrackExportWorker::OverwriteAnswer::CANCEL);

    worker.run();
    EXPECT_TRUE(worker.wait(10000));

    EXPECT_EQ(-1, m_answerer->currentProgress());
    EXPECT_EQ(-1, m_answerer->currentProgressCount());

    // Both files should have been skipped because we canceled the process,
    // so the .ogg shouldn't exist at all.
    QFileInfo newfile1(m_exportDir.filePath(kOggTestFile));
    EXPECT_FALSE(newfile1.exists());

    QFileInfo newfile2(m_exportDir.filePath(kM4aAacTestFile));
    EXPECT_TRUE(newfile2.exists());
    EXPECT_EQ(0, newfile2.size());
}

TEST_F(TrackExporterTest, DedupeList) {
    // Create a track list with a duplicate track, see that it gets deduped.
    mixxx::FileInfo fileinfo1(m_testDataDir.filePath(kOggTestFile));
    TrackPointer track1(Track::newTemporary(mixxx::FileAccess(fileinfo1)));
    TrackPointer track2(Track::newTemporary(mixxx::FileAccess(fileinfo1)));

    // Set up the worker and answerer.
    TrackPointerList tracks;
    tracks.append(track1);
    tracks.append(track2);
    TrackExportWorker worker(m_exportDir.canonicalPath(), tracks);
    m_answerer.reset(new FakeOverwriteAnswerer(&worker));

    worker.run();
    EXPECT_TRUE(worker.wait(10000));

    EXPECT_EQ(1, m_answerer->currentProgress());
    EXPECT_EQ(1, m_answerer->currentProgressCount());

    // Both files should have been overwritten.
    QFileInfo newfile1(m_exportDir.filePath(kOggTestFile));
    EXPECT_TRUE(newfile1.exists());
    // Should only be one file
    QFileInfoList files =
                m_exportDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);
    EXPECT_EQ(1, files.size());
}

TEST_F(TrackExporterTest, MungeFilename) {
    // Create a track list with a duplicate track in a different location,
    // see that the name gets munged.
    mixxx::FileInfo fileinfo1(m_testDataDir.filePath(kOggTestFile));
    TrackPointer track1(Track::newTemporary(mixxx::FileAccess(fileinfo1)));

    // Create a file with the same name in a different place.  Its filename
    // should be munged and the file still copied.
    QDir tempPath(QDir::tempPath());
    QFile file2(tempPath.filePath(kOggTestFile));
    mixxx::FileInfo fileinfo2(file2);
    ASSERT_TRUE(file2.open(QIODevice::WriteOnly));
    file2.close();
    TrackPointer track2(Track::newTemporary(mixxx::FileAccess(fileinfo2)));

    // Set up the worker and answerer.
    TrackPointerList tracks;
    tracks.append(track1);
    tracks.append(track2);
    TrackExportWorker worker(m_exportDir.canonicalPath(), tracks);
    m_answerer.reset(new FakeOverwriteAnswerer(&worker));

    worker.run();
    EXPECT_TRUE(worker.wait(10000));

    EXPECT_EQ(2, m_answerer->currentProgress());
    EXPECT_EQ(2, m_answerer->currentProgressCount());

    QFileInfo newfile1(m_exportDir.filePath(kOggTestFile));
    EXPECT_TRUE(newfile1.exists());
    QFileInfo newfile2(m_exportDir.filePath("cover-test-øé~ł€˚-0001.ogg"));
    EXPECT_TRUE(newfile2.exists());

    // Remove the track we created.
    tempPath.remove(kOggTestFile);
}
