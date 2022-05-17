#include "mixer/deck.h"

#include <sndfile.h>

#include <QRegularExpression>
#include <vector>

#include "control/controlobject.h"
#include "moc_deck.cpp"
#include "sources/soundsourceproxy.h"
#include "tensorflow/c/c_api.h"

namespace {

const QRegularExpression kDeckRegex(QStringLiteral("^\\[Channel(\\d+)\\]$"));

int extractIntFromRegex(const QRegularExpression& regex, const QString& group) {
    const QRegularExpressionMatch match = regex.match(group);
    DEBUG_ASSERT(match.isValid());
    if (!match.hasMatch()) {
        return false;
    }
    // The regex is expected to contain a single capture group with the number
    constexpr int capturedNumberIndex = 1;
    DEBUG_ASSERT(match.lastCapturedIndex() <= capturedNumberIndex);
    if (match.lastCapturedIndex() < capturedNumberIndex) {
        qWarning() << "No number found in group" << group;
        return false;
    }
    const QString capturedNumber = match.captured(capturedNumberIndex);
    DEBUG_ASSERT(!capturedNumber.isNull());
    bool okay = false;
    const int numberFromMatch = capturedNumber.toInt(&okay);
    VERIFY_OR_DEBUG_ASSERT(okay) {
        return false;
    }
    return numberFromMatch;
}

} //anonymous namespace

Deck::Deck(PlayerManager* pParent,
        UserSettingsPointer pConfig,
        EngineMaster* pMixingEngine,
        EffectsManager* pEffectsManager,
        EngineChannel::ChannelOrientation defaultOrientation,
        const ChannelHandleAndGroup& handleGroup)
        : BaseTrackPlayerImpl(pParent,
                  pConfig,
                  pMixingEngine,
                  pEffectsManager,
                  defaultOrientation,
                  handleGroup,
                  /*defaultMaster*/ true,
                  /*defaultHeadphones*/ false,
                  /*primaryDeck*/ true) {
    deckName = handleGroup.name();

    // Utilize half of the available cores for stem separation
    uint8_t kNumberOfSeparationThreads = math_max(1, QThread::idealThreadCount() / 2);

    uint8_t intra_op_parallelism_threads = kNumberOfSeparationThreads;
    uint8_t inter_op_parallelism_threads = kNumberOfSeparationThreads;
    uint8_t configSession[] = {0x10,
            intra_op_parallelism_threads,
            0x28,
            inter_op_parallelism_threads};

    TF_SetConfig(SessionOpts, configSession, sizeof configSession, status);

    QString modelPath = pConfig->getResourcePath() + QString("spleeter/5stems/");
    std::string modelPathString = modelPath.toStdString();
    const char* path = modelPathString.c_str();

    session = TF_LoadSessionFromSavedModel(
            SessionOpts, RunOpts, path, &tags, ntags, graph, NULL, status);
    if (TF_GetCode(status) != TF_OK) {
        std::cout << "ERROR: LoadSessionFromSavedModel: " << TF_Message(status) << std::endl;
    }

    m_pStemControl = new ControlObject(ConfigKey(handleGroup.name(), "LoadStems"));
    m_pStemControl->connectValueChangeRequest(this,
            &Deck::slotStemEnabled,
            Qt::DirectConnection);
}

Deck::~Deck() {
    TF_DeleteSessionOptions(SessionOpts);
    TF_DeleteSession(session, status);
    TF_DeleteStatus(status);
    delete m_pStemControl;
}

void Deck::Deallocator(void* data, size_t length, void* arg) {
    //free(data);
    //data = nullptr;
}

void Deck::slotStemEnabled(double v) {
    bool enable = v > 0.0;

    if (enable) {
        QThread* threadTF = QThread::create(threadedTensorflow, this);
        connect(threadTF, &QThread::finished, threadTF, &QThread::deleteLater);
        threadTF->start();
    }
}

void Deck::threadedTensorflow(Deck* deck) {
    const int deckNumber = extractIntFromRegex(kDeckRegex, deck->deckName);
    QString firstStemNumber;

    if (deckNumber == 1) {
        firstStemNumber = "1";
    }

    else if (deckNumber == 2) {
        firstStemNumber = "6";
    }

    else if (deckNumber == 3) {
        firstStemNumber = "11";
    }

    else if (deckNumber == 4) {
        firstStemNumber = "16";
    }

    TrackPointer pTrack = deck->getLoadedTrack();
    SoundSourceProxy proxy(pTrack);
    mixxx::AudioSourcePointer pAudioSource = proxy.openAudioSource();
    mixxx::SampleBuffer sampleBuffer(pAudioSource->frameLength() * 2);
    mixxx::ReadableSampleFrames readableSampleFrames =
            pAudioSource->readSampleFrames(
                    mixxx::WritableSampleFrames(
                            pAudioSource->frameIndexRange(),
                            mixxx::SampleBuffer::WritableSlice(sampleBuffer)));
    const CSAMPLE* framesToSpleet = readableSampleFrames.readableData();
    float* framesToSpleetCast = const_cast<float*>(framesToSpleet);
    float* framesToSpleetCastSoft = (float*)malloc(pAudioSource->frameLength() * 2 * sizeof(float));

    for (long int i = 0; i < pAudioSource->frameLength() * 2; i++) {
        framesToSpleetCastSoft[i] = framesToSpleetCast[i] / 2;
    }

    SNDFILE* out_file1;
    SNDFILE* out_file2;
    SNDFILE* out_file3;
    SNDFILE* out_file4;
    SNDFILE* out_file5;
    SF_INFO sfinfo;

    sfinfo.samplerate = 44100;
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    sfinfo.channels = 2;

    std::vector<TF_Output> input_tensors, output_tensors;
    std::vector<TF_Tensor*> input_values, output_values;

    //input tensor shape.
    std::int64_t input_dims[2] = {pAudioSource->frameLength(), 2};
    int num_bytes_in = pAudioSource->frameLength() * 2 * sizeof(float);

    input_tensors.push_back({TF_GraphOperationByName(deck->graph, "Placeholder"), 0});
    input_values.push_back(TF_NewTensor(TF_FLOAT,
            input_dims,
            deck->num_dims,
            framesToSpleetCastSoft,
            num_bytes_in,
            reinterpret_cast<void (*)(void*, size_t, void*)>(
                    &Deck::Deallocator),
            0));

    output_tensors.push_back({TF_GraphOperationByName(deck->graph, "strided_slice_50"), 0});
    output_values.push_back(nullptr);

    output_tensors.push_back({TF_GraphOperationByName(deck->graph, "strided_slice_52"), 0});
    output_values.push_back(nullptr);

    output_tensors.push_back({TF_GraphOperationByName(deck->graph, "strided_slice_54"), 0});
    output_values.push_back(nullptr);

    output_tensors.push_back({TF_GraphOperationByName(deck->graph, "strided_slice_56"), 0});
    output_values.push_back(nullptr);

    output_tensors.push_back({TF_GraphOperationByName(deck->graph, "strided_slice_58"), 0});
    output_values.push_back(nullptr);

    TF_SessionRun(deck->session,
            nullptr,
            &input_tensors[0],
            &input_values[0],
            input_values.size(),
            &output_tensors[0],
            &output_values[0],
            5,
            nullptr,
            0,
            nullptr,
            deck->status);

    if (TF_GetCode(deck->status) != TF_OK) {
        std::cout << "ERROR: SessionRun: " << TF_Message(deck->status) << std::endl;
    }

    QString firstScratchFile = QDir::homePath() + QString("/spleeterScratch_") +
            QString::number(firstStemNumber.toInt()) + QString(".wav");
    QString secondScratchFile = QDir::homePath() +
            QString("/spleeterScratch_") +
            QString::number(firstStemNumber.toInt() + 1) + QString(".wav");
    QString thirdScratchFile = QDir::homePath() + QString("/spleeterScratch_") +
            QString::number(firstStemNumber.toInt() + 2) + QString(".wav");
    QString fourthScratchFile = QDir::homePath() +
            QString("/spleeterScratch_") +
            QString::number(firstStemNumber.toInt() + 3) + QString(".wav");
    QString fifthScratchFile = QDir::homePath() + QString("/spleeterScratch_") +
            QString::number(firstStemNumber.toInt() + 4) + QString(".wav");

    out_file1 = sf_open(firstScratchFile.toStdString().c_str(), SFM_WRITE, &sfinfo);
    out_file2 = sf_open(secondScratchFile.toStdString().c_str(), SFM_WRITE, &sfinfo);
    out_file3 = sf_open(thirdScratchFile.toStdString().c_str(), SFM_WRITE, &sfinfo);
    out_file4 = sf_open(fourthScratchFile.toStdString().c_str(), SFM_WRITE, &sfinfo);
    out_file5 = sf_open(fifthScratchFile.toStdString().c_str(), SFM_WRITE, &sfinfo);

    sf_write_float(out_file1,
            (float*)TF_TensorData(output_values[0]),
            pAudioSource->frameLength() * 2);
    sf_write_float(out_file2,
            (float*)TF_TensorData(output_values[1]),
            pAudioSource->frameLength() * 2);
    sf_write_float(out_file3,
            (float*)TF_TensorData(output_values[2]),
            pAudioSource->frameLength() * 2);
    sf_write_float(out_file4,
            (float*)TF_TensorData(output_values[3]),
            pAudioSource->frameLength() * 2);
    sf_write_float(out_file5,
            (float*)TF_TensorData(output_values[4]),
            pAudioSource->frameLength() * 2);

    sf_close(out_file1);
    sf_close(out_file2);
    sf_close(out_file3);
    sf_close(out_file4);
    sf_close(out_file5);

    deck->m_pPlayerManager->slotLoadToStem(firstScratchFile, firstStemNumber.toInt());
    deck->m_pPlayerManager->slotLoadToStem(secondScratchFile, firstStemNumber.toInt() + 1);
    deck->m_pPlayerManager->slotLoadToStem(thirdScratchFile, firstStemNumber.toInt() + 2);
    deck->m_pPlayerManager->slotLoadToStem(fourthScratchFile, firstStemNumber.toInt() + 3);
    deck->m_pPlayerManager->slotLoadToStem(fifthScratchFile, firstStemNumber.toInt() + 4);

    free(framesToSpleetCastSoft);
    TF_DeleteTensor(input_values[0]);
    TF_DeleteTensor(output_values[0]);
    TF_DeleteTensor(output_values[1]);
    TF_DeleteTensor(output_values[2]);
    TF_DeleteTensor(output_values[3]);
    TF_DeleteTensor(output_values[4]);
}
