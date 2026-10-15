#pragma once

#include <QByteArray>
#include <QHash>
#include <QObject>
#include <QSet>
#include <QString>
#include <QTimer>

#include "preferences/usersettings.h"

class QUdpSocket;

/// Minimal Open Sound Control (OSC) endpoint that lets external tools and AI
/// agents drive Mixxx.
///
/// Addressing uses the Mixxx control naming scheme, i.e. the OSC address
/// "/Group/control" maps to the ControlObject "[Group],control". Examples:
///
///   /Channel1/play          1.0    start deck 1
///   /Channel1/rate          0.05   nudge the pitch fader
///   /Master/crossfader      -1.0   move the crossfader
///   /Channel2/sync_enabled  1.0    enable sync on deck 2
///
/// A float or 32 bit integer argument sets the control. Feedback is opt-in per
/// control: once a control has been written its value is streamed back to the
/// client whenever it changes, so a client can track the deck state.
class OscServer : public QObject {
    Q_OBJECT
  public:
    explicit OscServer(UserSettingsPointer pConfig, QObject* parent = nullptr);
    ~OscServer() override;

    /// Starts listening if enabled in the [OSC] settings group.
    void start();
    void stop();

    bool isListening() const;

    /// Converts an OSC address such as "/Channel1/play" into the equivalent
    /// Mixxx "[group],control" keys. Returns false for malformed addresses.
    static bool splitOscAddress(
            const QString& address, QString* pGroup, QString* pControl);

  private slots:
    void slotReadPendingDatagrams();
    void slotSendFeedback();

  private:
    void handleMessage(const QByteArray& datagram);
    void sendControl(const QString& address, double value);

    UserSettingsPointer m_pConfig;

    QUdpSocket* m_pSocket;

    QTimer m_feedbackTimer;

    // Addresses that a client has written; their values are streamed back.
    QSet<QString> m_subscribedAddresses;
    QHash<QString, double> m_lastFeedbackValues;

    quint16 m_outputPort;
    QString m_outputHost;
};
