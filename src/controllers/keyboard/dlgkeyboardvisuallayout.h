#pragma once

#include <QDialog>
#include <QMap>
#include <QKeySequence>
#include <QColor>
#include <QSharedPointer>

#include "preferences/configobject.h"

class KeyboardMappingManager;

/// A dialog that displays a graphical representation of the keyboard
/// with color-coded mappings.
class DlgKeyboardVisualLayout : public QDialog {
    Q_OBJECT

  public:
    explicit DlgKeyboardVisualLayout(QWidget* parent,
                                    ConfigObject<ConfigValueKbd>* pMapping,
                                    class KeyboardEventFilter* pFilter = nullptr);
    ~DlgKeyboardVisualLayout() override;

    /// Update the layout based on the current mapping
    void updateMapping(ConfigObject<ConfigValueKbd>* pMapping);

signals:
    void keyClicked(const QKeySequence& seq);

  public slots:
    void slotKeyPressedFromHardware(const QKeySequence& seq);
    void slotToggleKeyLearn(bool enabled);

  protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

  private:
    struct KeyInfo {
        QString label;
        QRectF rect;
        QKeySequence keySeq;
    };

    struct ActionPopup {
        QPointF pos;
        QString text;
        int life; // 0-100
    };

    struct KeyPulse {
        QPointF center;
        int radius;
        int life;
    };

    void initializeKeys();
    void renderKey(QPainter& painter, const KeyInfo& key, int index);
    QColor getColorForMapping(const ConfigKey& mapping);

    ConfigObject<ConfigValueKbd>* m_pMapping;
    QList<KeyInfo> m_keys;
    QMap<QKeySequence, ConfigKey> m_seqToConfig;

    // Animation state
    int m_highlightedKeyIndex;
    int m_highlightAlpha;
    QTimer* m_animTimer;
    
    class KeyboardEventFilter* m_pFilter;
    bool m_keyLearnActive;
    QPushButton* m_pKeyLearnButton;
    
    QList<ActionPopup> m_popups;
    QList<KeyPulse> m_pulses;
    
    int m_themeIndex;

  private slots:
    void slotUpdateAnimation();
};
