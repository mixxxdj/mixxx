#include "network/rest/certificategenerator.h"

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QProcess>
#include <QSslSocket>

#include "util/logger.h"

namespace mixxx::network::rest {

namespace {
constexpr int kCertificateLifetimeDays = 825; // OpenSSL default for self-signed certs
const Logger kLogger("mixxx::network::rest::CertificateGenerator");
} // namespace

CertificateGenerator::CertificateGenerator(QString settingsPath)
        : m_settingsPath(std::move(settingsPath)) {
}

CertificateGenerator::Result CertificateGenerator::loadOrGenerate(
        const QString& certificatePath,
        const QString& privateKeyPath,
        bool autoGenerate) const {
    const QString resolvedCertificatePath = certificatePath.isEmpty()
            ? defaultCertificatePath()
            : certificatePath;
    const QString resolvedPrivateKeyPath = privateKeyPath.isEmpty()
            ? defaultPrivateKeyPath()
            : privateKeyPath;

    Result result = loadCertificatePair(resolvedCertificatePath, resolvedPrivateKeyPath);
    if (result.success) {
        if (isExpired(result.certificate)) {
            result = Result{
                    .success = false,
                    .generated = false,
                    .certificatePath = resolvedCertificatePath,
                    .privateKeyPath = resolvedPrivateKeyPath,
                    .error = QObject::tr("REST TLS certificate is expired"),
            };
        }
    }

    if (result.success || !autoGenerate) {
        return result;
    }

    return generateCertificatePair(resolvedCertificatePath, resolvedPrivateKeyPath);
}

CertificateGenerator::Result CertificateGenerator::loadCertificatePair(
        const QString& certificatePath,
        const QString& privateKeyPath) const {
    QFile certificateFile(certificatePath);
    QFile privateKeyFile(privateKeyPath);
    if (!certificateFile.exists() || !privateKeyFile.exists()) {
        return fail(QObject::tr("Certificate or key file is missing"),
                certificatePath,
                privateKeyPath);
    }
    if (!certificateFile.open(QIODevice::ReadOnly)) {
        return fail(QObject::tr("Unable to open certificate file"),
                certificatePath,
                privateKeyPath);
    }
    if (!privateKeyFile.open(QIODevice::ReadOnly)) {
        return fail(QObject::tr("Unable to open private key file"),
                certificatePath,
                privateKeyPath);
    }

    const QSslCertificate certificate(certificateFile.readAll(), QSsl::Pem);
    const QSslKey privateKey(privateKeyFile.readAll(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
    const bool certificateValid = !certificate.isNull();
    const bool privateKeyValid = !privateKey.isNull();
    if (!certificateValid || !privateKeyValid) {
        if (!certificateValid && !privateKeyValid) {
            return fail(QObject::tr("Certificate and private key are invalid"),
                    certificatePath,
                    privateKeyPath);
        }
        if (!certificateValid) {
            return fail(QObject::tr("Certificate is invalid"),
                    certificatePath,
                    privateKeyPath);
        }
        return fail(QObject::tr("Private key is invalid"),
                certificatePath,
                privateKeyPath);
    }
    if (isExpired(certificate)) {
        return fail(QObject::tr("Certificate is expired"), certificatePath, privateKeyPath);
    }
    return finalizeResult(certificate, privateKey, certificatePath, privateKeyPath, false);
}

CertificateGenerator::Result CertificateGenerator::generateCertificatePair(
        const QString& certificatePath,
        const QString& privateKeyPath) const {
#if QT_CONFIG(ssl)
    if (!ensureDirectoryExists(certificatePath) || !ensureDirectoryExists(privateKeyPath)) {
        return fail(QObject::tr("Could not create certificate directory"),
                certificatePath,
                privateKeyPath);
    }

    // Clear out existing files if present.
    QFile::remove(certificatePath);
    QFile::remove(privateKeyPath);

    QProcess openssl;
    QStringList args{
            QStringLiteral("req"),
            QStringLiteral("-x509"),
            QStringLiteral("-newkey"),
            QStringLiteral("rsa:2048"),
            QStringLiteral("-nodes"),
            QStringLiteral("-subj"),
            QStringLiteral("/CN=Mixxx REST API"),
            QStringLiteral("-keyout"),
            privateKeyPath,
            QStringLiteral("-out"),
            certificatePath,
            QStringLiteral("-days"),
            QString::number(kCertificateLifetimeDays),
            QStringLiteral("-addext"),
            QStringLiteral("subjectAltName=DNS:localhost,IP:127.0.0.1,IP:0.0.0.0"),
    };
    openssl.start(QStringLiteral("openssl"), args);
    if (!openssl.waitForStarted(5000)) {
        return fail(QObject::tr("OpenSSL is not available for certificate generation"),
                certificatePath,
                privateKeyPath);
    }
    if (!openssl.waitForFinished(10000)) {
        return fail(QObject::tr("Timed out while generating certificate with OpenSSL"),
                certificatePath,
                privateKeyPath);
    }
    if (openssl.exitStatus() != QProcess::NormalExit || openssl.exitCode() != 0) {
        const QString errorOutput = QString::fromUtf8(openssl.readAllStandardError());
        kLogger.warning() << "OpenSSL certificate generation failed:" << errorOutput;
        return fail(QObject::tr("OpenSSL failed to generate certificate"),
                certificatePath,
                privateKeyPath);
    }

    QFile::setPermissions(certificatePath, QFile::ReadOwner | QFile::WriteOwner);
    QFile::setPermissions(privateKeyPath, QFile::ReadOwner | QFile::WriteOwner);

    Result result = loadCertificatePair(certificatePath, privateKeyPath);
    if (result.success) {
        result.generated = true;
    }
    return result;
#else
    Q_UNUSED(certificatePath);
    Q_UNUSED(privateKeyPath);
    return Result{
            .success = false,
            .generated = false,
            .error = QObject::tr("Qt was built without SSL support"),
    };
#endif
}

QString CertificateGenerator::defaultCertificatePath() const {
    return QDir(m_settingsPath).filePath(QStringLiteral("rest/rest_certificate.pem"));
}

QString CertificateGenerator::defaultPrivateKeyPath() const {
    return QDir(m_settingsPath).filePath(QStringLiteral("rest/rest_private_key.pem"));
}

bool CertificateGenerator::ensureDirectoryExists(const QString& path) const {
    const QFileInfo info(path);
    QDir directory = info.dir();
    if (directory.exists()) {
        QFile::setPermissions(directory.absolutePath(),
                QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
        return true;
    }
    if (!directory.mkpath(QStringLiteral("."))) {
        return false;
    }
    QFile::setPermissions(directory.absolutePath(),
            QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
    return true;
}

bool CertificateGenerator::isExpired(const QSslCertificate& certificate) {
    const QDateTime now = QDateTime::currentDateTimeUtc();
    return certificate.expiryDate() <= now || certificate.effectiveDate() > now;
}

CertificateGenerator::Result CertificateGenerator::fail(
        QString message, const QString& certificatePath, const QString& privateKeyPath) {
    return Result{
            .success = false,
            .generated = false,
            .certificatePath = certificatePath,
            .privateKeyPath = privateKeyPath,
            .error = std::move(message),
    };
}

CertificateGenerator::Result CertificateGenerator::finalizeResult(
        QSslCertificate certificate,
        QSslKey privateKey,
        const QString& certificatePath,
        const QString& privateKeyPath,
        bool generated) {
    return Result{
            .success = true,
            .generated = generated,
            .certificatePath = certificatePath,
            .privateKeyPath = privateKeyPath,
            .certificate = std::move(certificate),
            .privateKey = std::move(privateKey),
    };
}

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
