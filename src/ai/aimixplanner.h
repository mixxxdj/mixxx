#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QUrl>

#include <functional>

#include "preferences/usersettings.h"

class QNetworkAccessManager;
class QNetworkReply;

namespace mixxx {
namespace ai {

/// Configuration of the optional AI set planner.
///
/// The API key is deliberately kept out of this class. It is stored in the
/// system keychain by storeApiKey() and is never written to mixxx.cfg in
/// plain text.
class AiMixSettings final {
  public:
    enum class Provider {
        DeepSeek,
        OpenAI,
        Custom,
    };

    /// Stable identifier, also used to build the keychain entry name.
    static QString providerId(Provider provider);
    static QString providerDisplayName(Provider provider);
    static QString defaultBaseUrl(Provider provider);
    static QString defaultModel(Provider provider);

    /// Defaults used when the user has not configured anything.
    static constexpr int kDefaultMaxCandidateTracks = 300;
    static constexpr int kDefaultTimeoutSeconds = 120;

    static AiMixSettings fromUserSettings(const UserSettings& config);
    void writeToUserSettings(UserSettings* pConfig) const;

    /// Absolute URL of the OpenAI compatible chat completions endpoint.
    QUrl chatCompletionsUrl() const;

    Provider provider = Provider::DeepSeek;
    QString baseUrl;
    QString model;
    /// Upper bound on the number of tracks sent to the service. Keeps the
    /// request below the context limit and the cost predictable.
    int maxCandidateTracks = kDefaultMaxCandidateTracks;
    int timeoutSeconds = kDefaultTimeoutSeconds;
    bool enabled = false;
};

/// Secure storage for API keys.
///
/// When Mixxx was built without QtKeychain support these functions report that
/// storage is unavailable instead of silently writing the key to the regular
/// configuration file.
bool isApiKeyStorageAvailable();
/// Returns an empty string if no key is stored.
QString loadApiKey(const QString& providerId);
/// Stores a key. An empty key removes the stored entry.
/// Returns false and sets pError on failure.
bool storeApiKey(const QString& providerId, const QString& apiKey, QString* pError);

/// Extracts the "error.message" field of an API error response.
/// Returns false when the body does not carry a usable error message.
bool extractServiceErrorMessage(const QByteArray& responseBody, QString* pMessage);

/// A single candidate track as it is presented to the AI service.
struct AiCandidateTrack final {
    int index = 0;
    QString title;
    QString artist;
    QString genre;
    double bpm = 0.0;
    QString key;
    int durationSeconds = 0;

    QJsonObject toJson() const;
};

/// Asks an OpenAI compatible chat completions endpoint to turn a free form
/// instruction such as "build me a one hour techno set" into an ordered set
/// list of the candidate tracks the caller provides.
///
/// The service only decides *which* track is played *when*. Playing the
/// resulting queue and performing the actual transitions is left to Auto DJ.
class AiMixPlanner : public QObject {
    Q_OBJECT
  public:
    AiMixPlanner(QObject* pParent, UserSettingsPointer pConfig);
    ~AiMixPlanner() override;

    /// Body of an OpenAI compatible /chat/completions request.
    static QJsonObject buildRequestPayload(const AiMixSettings& settings,
            const QString& instruction,
            const QJsonArray& candidateTracks);

    /// Ordered candidate indices contained in a chat completions response.
    /// Returns an empty list and sets pError when the response cannot be used.
    static QList<int> parseTrackOrder(const QByteArray& responseBody,
            int candidateCount,
            QString* pError);

    /// Returns false and sets pError when no request can be sent.
    bool requestSetList(const QString& instruction,
            const QJsonArray& candidateTracks,
            QString* pError);

    /// Cancels a running request without emitting failed().
    void abort();

    bool isBusy() const;

  signals:
    void succeeded(const QList<int>& trackOrder);
    void failed(const QString& errorMessage);

  private:
    void handleReply();
    /// Looks the API key up without blocking and invokes onLoaded with an empty
    /// string when no key is stored or the keychain is unavailable.
    void readApiKeyAsync(const QString& providerId,
            std::function<void(const QString& apiKey)> onLoaded);
    /// Sends the chat completions request once the API key is known.
    void sendRequest(const AiMixSettings& settings,
            const QString& instruction,
            const QJsonArray& candidateTracks,
            const QString& apiKey);

    UserSettingsPointer m_pConfig;
    /// Created on the first request, so that no network backend is set up while
    /// the application is still starting up.
    QNetworkAccessManager* m_pNetworkAccessManager = nullptr;
    QPointer<QNetworkReply> m_pReply;
    /// True while the keychain is being queried. Keeps isBusy() true during the
    /// phase before the network request has been sent.
    bool m_apiKeyRequestPending = false;
    int m_candidateCount = 0;
};

} // namespace ai
} // namespace mixxx
