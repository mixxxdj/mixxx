#include "ai/aimixplanner.h"

#include "moc_aimixplanner.cpp"

#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSet>
#include <QUrl>

#include <utility>

#include "preferences/configobject.h"
#include "util/assert.h"

#ifdef __QTKEYCHAIN__
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <qt6keychain/keychain.h>
#else
#include <qt5keychain/keychain.h>
#endif
using namespace QKeychain;
#endif // __QTKEYCHAIN__

namespace mixxx {
namespace ai {

namespace {

const QString kConfigGroup = QStringLiteral("[AiAssistant]");
const QString kProviderKey = QStringLiteral("Provider");
const QString kBaseUrlKey = QStringLiteral("BaseUrl");
const QString kModelKey = QStringLiteral("Model");
const QString kEnabledKey = QStringLiteral("Enabled");
const QString kMaxCandidateTracksKey = QStringLiteral("MaxCandidateTracks");
const QString kTimeoutKey = QStringLiteral("TimeoutSeconds");

const QString kProviderIdDeepSeek = QStringLiteral("DeepSeek");
const QString kProviderIdOpenAI = QStringLiteral("OpenAI");
const QString kProviderIdCustom = QStringLiteral("Custom");

const QString kKeychainService = QStringLiteral("Mixxx - AI Assistant");

QString keychainKeyForProvider(const QString& providerId) {
    return QStringLiteral("ApiKey-") + providerId;
}

/// Instructions handed to the service. It is intentionally explicit about the
/// harmonic rules, because the model has no access to Mixxx itself.
const QString kSystemPrompt = QStringLiteral(
        "You are an experienced club DJ who plans harmonic DJ sets.\n"
        "You receive a JSON array of candidate tracks. Each entry has an "
        "\"index\" (the number you must refer to), \"title\", \"artist\", "
        "\"genre\", \"bpm\", \"key\" and \"duration\" in seconds. \"key\" uses "
        "Camelot notation, so 8A mixes well with 8A, 8B, 7A and 9A.\n"
        "Rules:\n"
        "1. Honour the user's request, especially genre, energy and total length.\n"
        "2. Order the tracks so that consecutive tracks mix well: same or "
        "adjacent Camelot key and a tempo difference of at most 6 percent.\n"
        "3. Never repeat a track and only use indices that were provided.\n"
        "4. Answer with just a JSON array of indices in play order, for example "
        "[12, 4, 27]. Add no explanation.");

} // namespace

QString AiMixSettings::providerId(Provider provider) {
    switch (provider) {
    case Provider::DeepSeek:
        return kProviderIdDeepSeek;
    case Provider::OpenAI:
        return kProviderIdOpenAI;
    case Provider::Custom:
        break;
    }
    return kProviderIdCustom;
}

QString AiMixSettings::providerDisplayName(Provider provider) {
    switch (provider) {
    case Provider::DeepSeek:
        return QStringLiteral("DeepSeek");
    case Provider::OpenAI:
        return QStringLiteral("OpenAI");
    case Provider::Custom:
        break;
    }
    return QObject::tr("Compatible with OpenAI (local or third party API)");
}

QString AiMixSettings::defaultBaseUrl(Provider provider) {
    switch (provider) {
    case Provider::DeepSeek:
        return QStringLiteral("https://api.deepseek.com/v1");
    case Provider::OpenAI:
        return QStringLiteral("https://api.openai.com/v1");
    case Provider::Custom:
        break;
    }
    // A local Ollama instance exposes an OpenAI compatible API on this port,
    // so the default keeps the data on the user's own machine.
    return QStringLiteral("http://localhost:11434/v1");
}

QString AiMixSettings::defaultModel(Provider provider) {
    switch (provider) {
    case Provider::DeepSeek:
        return QStringLiteral("deepseek-chat");
    case Provider::OpenAI:
        return QStringLiteral("gpt-4o-mini");
    case Provider::Custom:
        break;
    }
    return QString();
}

AiMixSettings AiMixSettings::fromUserSettings(const UserSettings& config) {
    AiMixSettings settings;

    const QString providerIdValue = config.getValueString(ConfigKey(kConfigGroup, kProviderKey));
    if (providerIdValue == kProviderIdOpenAI) {
        settings.provider = Provider::OpenAI;
    } else if (providerIdValue == kProviderIdCustom) {
        settings.provider = Provider::Custom;
    } else {
        settings.provider = Provider::DeepSeek;
    }

    settings.baseUrl = config.getValueString(ConfigKey(kConfigGroup, kBaseUrlKey)).trimmed();
    if (settings.baseUrl.isEmpty()) {
        settings.baseUrl = defaultBaseUrl(settings.provider);
    }

    settings.model = config.getValueString(ConfigKey(kConfigGroup, kModelKey)).trimmed();
    if (settings.model.isEmpty()) {
        settings.model = defaultModel(settings.provider);
    }

    settings.enabled = config.getValue(ConfigKey(kConfigGroup, kEnabledKey), false);
    settings.maxCandidateTracks = config.getValue(ConfigKey(kConfigGroup, kMaxCandidateTracksKey),
            kDefaultMaxCandidateTracks);
    if (settings.maxCandidateTracks <= 0) {
        settings.maxCandidateTracks = kDefaultMaxCandidateTracks;
    }
    settings.timeoutSeconds = config.getValue(ConfigKey(kConfigGroup, kTimeoutKey),
            kDefaultTimeoutSeconds);
    if (settings.timeoutSeconds <= 0) {
        settings.timeoutSeconds = kDefaultTimeoutSeconds;
    }
    return settings;
}

void AiMixSettings::writeToUserSettings(UserSettings* pConfig) const {
    DEBUG_ASSERT(pConfig != nullptr);
    pConfig->setValue(ConfigKey(kConfigGroup, kProviderKey), providerId(provider));
    pConfig->setValue(ConfigKey(kConfigGroup, kBaseUrlKey), baseUrl.trimmed());
    pConfig->setValue(ConfigKey(kConfigGroup, kModelKey), model.trimmed());
    pConfig->setValue(ConfigKey(kConfigGroup, kEnabledKey), enabled);
    pConfig->setValue(ConfigKey(kConfigGroup, kMaxCandidateTracksKey), maxCandidateTracks);
    pConfig->setValue(ConfigKey(kConfigGroup, kTimeoutKey), timeoutSeconds);
}

QUrl AiMixSettings::chatCompletionsUrl() const {
    QUrl url(baseUrl.trimmed());
    QString path = url.path();
    while (path.endsWith(QLatin1Char('/'))) {
        path.chop(1);
    }
    url.setPath(path + QStringLiteral("/chat/completions"));
    return url;
}

bool isApiKeyStorageAvailable() {
#ifdef __QTKEYCHAIN__
    return true;
#else
    return false;
#endif
}

QString loadApiKey(const QString& providerId) {
#ifdef __QTKEYCHAIN__
    ReadPasswordJob readJob(kKeychainService);
    readJob.setAutoDelete(false);
    readJob.setKey(keychainKeyForProvider(providerId));

    QEventLoop loop;
    QObject::connect(&readJob, &ReadPasswordJob::finished, &loop, &QEventLoop::quit);
    readJob.start();
    loop.exec();

    if (readJob.error() != NoError) {
        // A missing entry is the normal "not configured yet" case.
        return QString();
    }
    return readJob.textData();
#else
    Q_UNUSED(providerId);
    return QString();
#endif
}

bool storeApiKey(const QString& providerId, const QString& apiKey, QString* pError) {
#ifdef __QTKEYCHAIN__
    if (apiKey.isEmpty()) {
        DeletePasswordJob deleteJob(kKeychainService);
        deleteJob.setAutoDelete(false);
        deleteJob.setKey(keychainKeyForProvider(providerId));

        QEventLoop loop;
        QObject::connect(&deleteJob, &DeletePasswordJob::finished, &loop, &QEventLoop::quit);
        deleteJob.start();
        loop.exec();
        if (deleteJob.error() != NoError) {
            if (pError != nullptr) {
                *pError = deleteJob.errorString();
            }
            return false;
        }
        return true;
    }

    WritePasswordJob writeJob(kKeychainService);
    writeJob.setAutoDelete(false);
    writeJob.setKey(keychainKeyForProvider(providerId));
    writeJob.setTextData(apiKey);

    QEventLoop loop;
    QObject::connect(&writeJob, &WritePasswordJob::finished, &loop, &QEventLoop::quit);
    writeJob.start();
    loop.exec();
    if (writeJob.error() != NoError) {
        if (pError != nullptr) {
            *pError = writeJob.errorString();
        }
        return false;
    }
    return true;
#else
    Q_UNUSED(providerId);
    Q_UNUSED(apiKey);
    if (pError != nullptr) {
        *pError = QObject::tr(
                "This build of Mixxx was compiled without QtKeychain support, so the API key "
                "cannot be stored securely.");
    }
    return false;
#endif
}

bool extractServiceErrorMessage(const QByteArray& responseBody, QString* pMessage) {
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(responseBody, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return false;
    }
    const QJsonValue errorValue = document.object().value(QStringLiteral("error"));
    if (!errorValue.isObject()) {
        return false;
    }
    const QString message = errorValue.toObject().value(QStringLiteral("message")).toString();
    if (message.isEmpty()) {
        return false;
    }
    if (pMessage != nullptr) {
        *pMessage = message;
    }
    return true;
}

QJsonObject AiCandidateTrack::toJson() const {
    QJsonObject object;
    object.insert(QStringLiteral("index"), static_cast<double>(index));
    object.insert(QStringLiteral("title"), title);
    object.insert(QStringLiteral("artist"), artist);
    object.insert(QStringLiteral("genre"), genre);
    object.insert(QStringLiteral("bpm"), bpm);
    object.insert(QStringLiteral("key"), key);
    object.insert(QStringLiteral("duration"), static_cast<double>(durationSeconds));
    return object;
}

AiMixPlanner::AiMixPlanner(QObject* pParent, UserSettingsPointer pConfig)
        : QObject(pParent),
          m_pConfig(std::move(pConfig)) {
}

AiMixPlanner::~AiMixPlanner() = default;

bool AiMixPlanner::isBusy() const {
    return m_pReply != nullptr || m_apiKeyRequestPending;
}

QJsonObject AiMixPlanner::buildRequestPayload(const AiMixSettings& settings,
        const QString& instruction,
        const QJsonArray& candidateTracks) {
    QJsonObject systemMessage;
    systemMessage.insert(QStringLiteral("role"), QStringLiteral("system"));
    systemMessage.insert(QStringLiteral("content"), kSystemPrompt);

    // Assigned to a QString first: with QT_USE_QSTRINGBUILDER the chained
    // concatenation below yields a builder proxy, which QJsonValue cannot
    // take as a single implicit conversion.
    const QString userContent =
            QStringLiteral("Request: ") + instruction.trimmed() +
            QStringLiteral("\n\nCandidate tracks (JSON):\n") +
            QString::fromUtf8(QJsonDocument(candidateTracks).toJson(QJsonDocument::Compact));

    QJsonObject userMessage;
    userMessage.insert(QStringLiteral("role"), QStringLiteral("user"));
    userMessage.insert(QStringLiteral("content"), userContent);

    QJsonArray messages;
    messages.append(systemMessage);
    messages.append(userMessage);

    QJsonObject payload;
    payload.insert(QStringLiteral("model"), settings.model);
    payload.insert(QStringLiteral("messages"), messages);
    payload.insert(QStringLiteral("stream"), false);
    return payload;
}

QList<int> AiMixPlanner::parseTrackOrder(const QByteArray& responseBody,
        int candidateCount,
        QString* pError) {
    const auto setError = [pError](const QString& message) {
        if (pError != nullptr) {
            *pError = message;
        }
    };

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(responseBody, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        setError(tr("The AI service did not return valid JSON (%1).")
                         .arg(parseError.errorString()));
        return {};
    }

    const QJsonArray choices = document.object().value(QStringLiteral("choices")).toArray();
    if (choices.isEmpty()) {
        QString serviceMessage;
        if (extractServiceErrorMessage(responseBody, &serviceMessage)) {
            setError(serviceMessage);
        } else {
            setError(tr("The AI service response does not contain any completion."));
        }
        return {};
    }

    const QString content = choices.at(0)
                                    .toObject()
                                    .value(QStringLiteral("message"))
                                    .toObject()
                                    .value(QStringLiteral("content"))
                                    .toString();
    if (content.isEmpty()) {
        setError(tr("The AI service returned an empty completion."));
        return {};
    }

    // The model may wrap the array in prose or a markdown code fence, so the
    // first opening and the last closing bracket are used as the payload.
    const int start = content.indexOf(QLatin1Char('['));
    const int end = content.lastIndexOf(QLatin1Char(']'));
    if (start < 0 || end <= start) {
        setError(tr("The AI service did not return a list of tracks."));
        return {};
    }

    QJsonParseError arrayParseError;
    const QJsonDocument arrayDocument = QJsonDocument::fromJson(
            content.mid(start, end - start + 1).toUtf8(), &arrayParseError);
    if (arrayParseError.error != QJsonParseError::NoError || !arrayDocument.isArray()) {
        setError(tr("The track list returned by the AI service could not be read."));
        return {};
    }

    QList<int> order;
    QSet<int> seen;
    for (const auto& value : arrayDocument.array()) {
        if (!value.isDouble()) {
            continue;
        }
        const int index = value.toInt(-1);
        if (index < 0 || index >= candidateCount || seen.contains(index)) {
            continue;
        }
        seen.insert(index);
        order.append(index);
    }

    if (order.isEmpty()) {
        setError(tr("The AI service did not select any of the provided tracks."));
        return {};
    }
    return order;
}

void AiMixPlanner::readApiKeyAsync(const QString& providerId,
        std::function<void(const QString& apiKey)> onLoaded) {
#ifdef __QTKEYCHAIN__
    auto* pJob = new ReadPasswordJob(kKeychainService, this);
    pJob->setAutoDelete(true);
    pJob->setKey(keychainKeyForProvider(providerId));
    connect(pJob, &ReadPasswordJob::finished, this, [onLoaded, pJob]() {
        if (pJob->error() != NoError) {
            // A missing entry is the normal "not configured yet" case.
            onLoaded(QString());
            return;
        }
        onLoaded(pJob->textData());
    });
    pJob->start();
#else
    Q_UNUSED(providerId);
    onLoaded(QString());
#endif
}

void AiMixPlanner::sendRequest(const AiMixSettings& settings,
        const QString& instruction,
        const QJsonArray& candidateTracks,
        const QString& apiKey) {
    if (m_pNetworkAccessManager == nullptr) {
        m_pNetworkAccessManager = new QNetworkAccessManager(this);
    }

    QNetworkRequest request(settings.chatCompletionsUrl());
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", "Bearer " + apiKey.toUtf8());
    request.setTransferTimeout(settings.timeoutSeconds * 1000);

    m_candidateCount = candidateTracks.size();
    const QByteArray payload = QJsonDocument(
            buildRequestPayload(settings, instruction, candidateTracks))
                                       .toJson(QJsonDocument::Compact);
    QNetworkReply* pReply = m_pNetworkAccessManager->post(request, payload);
    m_pReply = pReply;
    connect(pReply, &QNetworkReply::finished, this, &AiMixPlanner::handleReply);
}

bool AiMixPlanner::requestSetList(const QString& instruction,
        const QJsonArray& candidateTracks,
        QString* pError) {
    const auto setError = [pError](const QString& message) {
        if (pError != nullptr) {
            *pError = message;
        }
        return false;
    };

    if (isBusy()) {
        return setError(tr("A request to the AI service is already running."));
    }
    if (instruction.trimmed().isEmpty()) {
        return setError(tr("Please enter what kind of set you want."));
    }
    if (candidateTracks.isEmpty()) {
        return setError(tr("There are no tracks to choose from."));
    }

    const AiMixSettings settings = AiMixSettings::fromUserSettings(*m_pConfig);
    if (!settings.enabled) {
        return setError(tr("The AI assistant is disabled in the preferences."));
    }
    if (settings.model.isEmpty()) {
        return setError(tr("No model is configured for the AI assistant."));
    }

    const QUrl url = settings.chatCompletionsUrl();
    if (!url.isValid() || url.host().isEmpty()) {
        return setError(tr("The API address configured for the AI assistant is not valid."));
    }

    if (!isApiKeyStorageAvailable()) {
        return setError(tr("No API key is stored for %1.")
                                .arg(AiMixSettings::providerDisplayName(settings.provider)));
    }

    // The key is read asynchronously. The blocking variant ran a local event
    // loop, which could re-enter the planner while a request was already in
    // flight.
    m_apiKeyRequestPending = true;
    readApiKeyAsync(AiMixSettings::providerId(settings.provider),
            [this, settings, instruction, candidateTracks](const QString& apiKey) {
                if (!m_apiKeyRequestPending) {
                    // abort() discarded the request, so nothing is emitted.
                    return;
                }
                m_apiKeyRequestPending = false;
                if (apiKey.isEmpty()) {
                    emit failed(tr("No API key is stored for %1.")
                                        .arg(AiMixSettings::providerDisplayName(
                                                settings.provider)));
                    return;
                }
                sendRequest(settings, instruction, candidateTracks, apiKey);
            });
    return true;
}

void AiMixPlanner::abort() {
    m_apiKeyRequestPending = false;
    QNetworkReply* pReply = m_pReply.data();
    if (pReply == nullptr) {
        return;
    }
    m_pReply = nullptr;
    pReply->abort();
    pReply->deleteLater();
}

void AiMixPlanner::handleReply() {
    QNetworkReply* pReply = m_pReply.data();
    if (pReply == nullptr) {
        // The request was cancelled by abort().
        return;
    }
    m_pReply = nullptr;

    const QByteArray body = pReply->readAll();
    const QNetworkReply::NetworkError networkError = pReply->error();
    const QString networkErrorString = pReply->errorString();
    pReply->deleteLater();

    if (networkError != QNetworkReply::NoError) {
        // The service usually explains the problem (invalid key, quota, ...)
        // more precisely than the generic Qt error does.
        QString serviceMessage;
        if (extractServiceErrorMessage(body, &serviceMessage)) {
            emit failed(serviceMessage);
        } else {
            emit failed(tr("Could not reach the AI service: %1").arg(networkErrorString));
        }
        return;
    }

    QString parseError;
    const QList<int> order = parseTrackOrder(body, m_candidateCount, &parseError);
    if (order.isEmpty()) {
        emit failed(parseError);
        return;
    }
    emit succeeded(order);
}

} // namespace ai
} // namespace mixxx
