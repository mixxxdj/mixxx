#pragma once

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslKey>

#include <optional>

namespace mixxx::network::rest {

class CertificateGenerator {
  public:
    struct Result {
        bool success{false};
        bool generated{false};
        QString certificatePath;
        QString privateKeyPath;
        QSslCertificate certificate;
        QSslKey privateKey;
        QString error;
    };

    explicit CertificateGenerator(QString settingsPath);

    Result loadOrGenerate(
            const QString& certificatePath,
            const QString& privateKeyPath,
            bool autoGenerate) const;

  private:
    static bool isExpired(const QSslCertificate& certificate);
    static Result fail(QString message, const QString& certificatePath, const QString& privateKeyPath);
    static Result finalizeResult(
            QSslCertificate certificate,
            QSslKey privateKey,
            const QString& certificatePath,
            const QString& privateKeyPath,
            bool generated);

    Result loadCertificatePair(const QString& certificatePath, const QString& privateKeyPath) const;
    Result generateCertificatePair(const QString& certificatePath, const QString& privateKeyPath) const;
    QString defaultCertificatePath() const;
    QString defaultPrivateKeyPath() const;
    bool ensureDirectoryExists(const QString& path) const;

    QString m_settingsPath;
};

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
