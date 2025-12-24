#include <QToolTip>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "controllers/keyboard/keyboardeventfilter.h"

#include "moc_dlgkeyboardvisuallayout.cpp"

DlgKeyboardVisualLayout::DlgKeyboardVisualLayout(QWidget* parent,
                                               ConfigObject<ConfigValueKbd>* pMapping,
                                               KeyboardEventFilter* pFilter)
        : QDialog(parent),
          m_pMapping(pMapping),
          m_pFilter(pFilter),
          m_highlightedKeyIndex(-1),
          m_highlightAlpha(0),
          m_keyLearnActive(false),
          m_themeIndex(0) {
    setWindowTitle(tr("Visual Keyboard Layout - Professional Suite"));
    setMinimumSize(950, 480);
    setMouseTracking(true);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Top Controls
    QHBoxLayout* controls = new QHBoxLayout();
    m_pKeyLearnButton = new QPushButton(tr("Enable Key Learn (Hardware Capture)"), this);
    m_pKeyLearnButton->setCheckable(true);
    m_pKeyLearnButton->setFocusPolicy(Qt::NoFocus);
    m_pKeyLearnButton->setStyleSheet("QPushButton { background: #333; color: white; padding: 8px; border-radius: 4px; }"
                                    "QPushButton:checked { background: #ff4444; font-weight: bold; }");
    connect(m_pKeyLearnButton, &QPushButton::toggled, this, &DlgKeyboardVisualLayout::slotToggleKeyLearn);
    controls->addWidget(m_pKeyLearnButton);
    controls->addStretch();
    
    QPushButton* pThemeButton = new QPushButton(tr("Change Theme"), this);
    pThemeButton->setFocusPolicy(Qt::NoFocus);
    pThemeButton->setStyleSheet("background: #444; color: white; padding: 5px;");
    connect(pThemeButton, &QPushButton::clicked, this, [this]() {
        m_themeIndex = (m_themeIndex + 1) % 4;
        update();
    });
    controls->addWidget(pThemeButton);
    controls->addSpacing(10);

    QLabel* helpLabel = new QLabel(tr("Click keys to map | Press Hardware for Echo"), this);
    helpLabel->setStyleSheet("color: #0080ff; font-weight: bold;");
    controls->addWidget(helpLabel);
    
    mainLayout->addLayout(controls);
    mainLayout->addStretch(1); 
    
    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(30);
    connect(m_animTimer, &QTimer::timeout, this, &DlgKeyboardVisualLayout::slotUpdateAnimation);

    if (m_pFilter) {
        connect(m_pFilter, &KeyboardEventFilter::keyPressed, 
                this, &DlgKeyboardVisualLayout::slotKeyPressedFromHardware);
    }

    initializeKeys();
    updateMapping(pMapping);
}

DlgKeyboardVisualLayout::~DlgKeyboardVisualLayout() {
}

void DlgKeyboardVisualLayout::updateMapping(ConfigObject<ConfigValueKbd>* pMapping) {
    m_pMapping = pMapping;
    m_seqToConfig.clear();
    
    // Invert mapping for easy lookup
    QMultiHash<ConfigValueKbd, ConfigKey> transposed = pMapping->transpose();
    for (auto it = transposed.begin(); it != transposed.end(); ++it) {
        // Normalize the string for reliable sequence lookup
        m_seqToConfig[QKeySequence(it.key().value, QKeySequence::PortableText)] = it.value();
    }
    update();
}

void DlgKeyboardVisualLayout::initializeKeys() {
    m_keys.clear();
    
    double keySize = 40.0;
    double padding = 2.0;
    
    // Total width of main keyboard is roughly 15 * (keySize + padding) + some offsets
    // Numpad starts at an offset
    double numpadX = padding + 15 * (keySize + padding) + 40;

    // Top Row: Esc + F-Keys
    double topPadding = 60.0; // Pushes keyboard down to avoid overlap
    KeyInfo esc;
    esc.label = "Esc";
    esc.keySeq = QKeySequence(Qt::Key_Escape);
    esc.rect = QRectF(padding, topPadding, keySize, keySize);
    m_keys.append(esc);

    for (int i = 0; i < 12; ++i) {
        KeyInfo key;
        key.label = QString("F%1").arg(i + 1);
        key.keySeq = QKeySequence(Qt::Key_F1 + i);
        double fOffset = keySize + 20 + i * (keySize + padding) + (i > 3 ? 10 : 0) + (i > 7 ? 10 : 0);
        key.rect = QRectF(fOffset, topPadding, keySize, keySize);
        m_keys.append(key);
    }

    // Main Row 1: ` 1 2 ... 0 - = Backspace
    double r1Y = topPadding + keySize + 20;
    QString r1 = "`1234567890-=";
    for (int i = 0; i < r1.length(); ++i) {
        KeyInfo key;
        key.label = r1[i];
        key.keySeq = QKeySequence(key.label);
        key.rect = QRectF(padding + i * (keySize + padding), r1Y, keySize, keySize);
        m_keys.append(key);
    }
    KeyInfo backspace;
    backspace.label = "Back";
    backspace.keySeq = QKeySequence(Qt::Key_Backspace);
    backspace.rect = QRectF(padding + 13 * (keySize + padding), r1Y, keySize * 2, keySize);
    m_keys.append(backspace);

    // Main Row 2: Tab Q W E ... [ ] \ (Backslash)
    double r2Y = r1Y + keySize + padding;
    KeyInfo tab;
    tab.label = "Tab";
    tab.keySeq = QKeySequence(Qt::Key_Tab);
    tab.rect = QRectF(padding, r2Y, keySize * 1.5, keySize);
    m_keys.append(tab);
    
    QString r2 = "QWERTYUIOP[]";
    for (int i = 0; i < r2.length(); ++i) {
        KeyInfo key;
        key.label = r2[i];
        key.keySeq = QKeySequence(key.label);
        key.rect = QRectF(padding + keySize * 1.5 + padding + i * (keySize + padding), r2Y, keySize, keySize);
        m_keys.append(key);
    }
    KeyInfo bslash;
    bslash.label = "\\";
    bslash.keySeq = QKeySequence("\\");
    bslash.rect = QRectF(padding + keySize * 1.5 + padding + 12 * (keySize + padding), r2Y, keySize * 1.5, keySize);
    m_keys.append(bslash);

    // Main Row 3: Caps A S D ... ' Enter
    double r3Y = r2Y + keySize + padding;
    KeyInfo caps;
    caps.label = "Caps";
    caps.keySeq = QKeySequence(Qt::Key_CapsLock);
    caps.rect = QRectF(padding, r3Y, keySize * 1.8, keySize);
    m_keys.append(caps);

    QString r3 = "ASDFGHJKL;'";
    for (int i = 0; i < r3.length(); ++i) {
        KeyInfo key;
        key.label = r3[i];
        key.keySeq = QKeySequence(key.label);
        key.rect = QRectF(padding + keySize * 1.8 + padding + i * (keySize + padding), r3Y, keySize, keySize);
        m_keys.append(key);
    }
    KeyInfo enter;
    enter.label = "Enter";
    enter.keySeq = QKeySequence(Qt::Key_Return);
    enter.rect = QRectF(padding + keySize * 1.8 + padding + 11 * (keySize + padding), r3Y, keySize * 2.2, keySize);
    m_keys.append(enter);

    // Main Row 4: L-Shift Z X C ... / R-Shift
    double r4Y = r3Y + keySize + padding;
    KeyInfo lShift;
    lShift.label = "Shift";
    lShift.keySeq = QKeySequence(Qt::Key_Shift);
    lShift.rect = QRectF(padding, r4Y, keySize * 2.4, keySize);
    m_keys.append(lShift);

    QString r4 = "ZXCVBNM,./";
    for (int i = 0; i < r4.length(); ++i) {
        KeyInfo key;
        key.label = r4[i];
        key.keySeq = QKeySequence(key.label);
        key.rect = QRectF(padding + keySize * 2.4 + padding + i * (keySize + padding), r4Y, keySize, keySize);
        m_keys.append(key);
    }
    KeyInfo rShift;
    rShift.label = "Shift";
    rShift.keySeq = QKeySequence(Qt::Key_Shift);
    rShift.rect = QRectF(padding + keySize * 2.4 + padding + 10 * (keySize + padding), r4Y, keySize * 2.6, keySize);
    m_keys.append(rShift);

    // Main Row 5: Ctrl Meta Alt Space Alt Meta Ctrl
    double r5Y = r4Y + keySize + padding;
    struct { QString label; Qt::Key key; } r5Mods[] = {
        {"Ctrl", Qt::Key_Control},
        {"Meta", Qt::Key_Meta},
        {"Alt", Qt::Key_Alt}
    };
    for (int i = 0; i < 3; ++i) {
        KeyInfo key;
        key.label = r5Mods[i].label;
        key.keySeq = QKeySequence(r5Mods[i].key);
        key.rect = QRectF(padding + i * (keySize * 1.5 + padding), r5Y, keySize * 1.5, keySize);
        m_keys.append(key);
    }
    KeyInfo space;
    space.label = "Space";
    space.keySeq = QKeySequence(Qt::Key_Space);
    space.rect = QRectF(padding + 3 * (keySize * 1.5 + padding), r5Y, keySize * 6.5, keySize);
    m_keys.append(space);

    // Arrow Keys (shifted right)
    double arrowX = padding + 15 * (keySize + padding);
    KeyInfo left;
    left.label = "←";
    left.keySeq = QKeySequence(Qt::Key_Left);
    left.rect = QRectF(arrowX, r5Y, keySize, keySize);
    m_keys.append(left);

    KeyInfo up;
    up.label = "↑";
    up.keySeq = QKeySequence(Qt::Key_Up);
    up.rect = QRectF(arrowX + keySize + padding, r4Y, keySize, keySize);
    m_keys.append(up);

    KeyInfo down;
    down.label = "↓";
    down.keySeq = QKeySequence(Qt::Key_Down);
    down.rect = QRectF(arrowX + keySize + padding, r5Y, keySize, keySize);
    m_keys.append(down);

    KeyInfo right;
    right.label = "→";
    right.keySeq = QKeySequence(Qt::Key_Right);
    right.rect = QRectF(arrowX + 2 * (keySize + padding), r5Y, keySize, keySize);
    m_keys.append(right);

    // Numpad
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            KeyInfo key;
            int num = (2 - i) * 3 + (j + 1);
            key.label = QString::number(num);
            key.keySeq = QKeySequence(QString::number(num));
            key.rect = QRectF(numpadX + j * (keySize + padding), r2Y + i * (keySize + padding), keySize, keySize);
            m_keys.append(key);
        }
    }
    KeyInfo num0;
    num0.label = "0";
    num0.keySeq = QKeySequence("0");
    num0.rect = QRectF(numpadX, r5Y, keySize * 2 + padding, keySize);
    m_keys.append(num0);
    
    KeyInfo numDot;
    numDot.label = ".";
    numDot.keySeq = QKeySequence(".");
    numDot.rect = QRectF(numpadX + 2 * (keySize + padding), r5Y, keySize, keySize);
    m_keys.append(numDot);

    KeyInfo numEnter;
    numEnter.label = "Enter";
    numEnter.keySeq = QKeySequence(Qt::Key_Enter);
    numEnter.rect = QRectF(numpadX + 3 * (keySize + padding), r4Y, keySize, keySize * 2 + padding);
    m_keys.append(numEnter);

    KeyInfo numPlus;
    numPlus.label = "+";
    numPlus.keySeq = QKeySequence("+");
    numPlus.rect = QRectF(numpadX + 3 * (keySize + padding), r2Y, keySize, keySize * 2 + padding);
    m_keys.append(numPlus);
}

void DlgKeyboardVisualLayout::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw background
    painter.fillRect(rect(), QColor(25, 25, 25));

    // Simple Scaling
    double scaleX = width() / 900.0;
    double scaleY = height() / 400.0;
    painter.scale(scaleX, scaleY);

    for (int i = 0; i < m_keys.size(); ++i) {
        renderKey(painter, m_keys[i], i);
    }

    // Draw Pulses
    for (const KeyPulse& pulse : std::as_const(m_pulses)) {
        QRadialGradient grad(pulse.center, pulse.radius);
        QColor c = (m_themeIndex == 0 ? QColor(0, 150, 255) : 
                   (m_themeIndex == 1 ? QColor(0, 255, 150) : 
                   (m_themeIndex == 2 ? QColor(255, 50, 50) : QColor(255, 200, 0))));
        c.setAlpha(pulse.life * 2);
        grad.setColorAt(0, c);
        grad.setColorAt(1, Qt::transparent);
        painter.setBrush(grad);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(pulse.center, pulse.radius, pulse.radius);
    }

    // Draw Popups
    for (const ActionPopup& popup : std::as_const(m_popups)) {
        painter.save();
        painter.translate(popup.pos.x(), popup.pos.y() - (100 - popup.life) * 0.5);
        QColor c(255, 255, 255);
        c.setAlpha(popup.life * 2.5);
        painter.setPen(c);
        QFont f = painter.font();
        f.setBold(true);
        f.setPointSize(10);
        painter.setFont(f);
        
        // Glow effect
        painter.setPen(QColor(0, 0, 0, popup.life));
        painter.drawText(QRectF(-101, -19, 200, 40), Qt::AlignCenter, popup.text);
        painter.setPen(c);
        painter.drawText(QRectF(-100, -20, 200, 40), Qt::AlignCenter, popup.text);
        painter.restore();
    }
}

void DlgKeyboardVisualLayout::renderKey(QPainter& painter, const KeyInfo& key, int index) {
    QColor baseColor(45, 45, 45);
    QColor textColor(180, 180, 180);
    QColor borderColor(80, 80, 80);

    if (m_seqToConfig.contains(key.keySeq)) {
        baseColor = getColorForMapping(m_seqToConfig[key.keySeq]);
        textColor = Qt::black;
        borderColor = baseColor.lighter(150);
    }

    // Apply animation highlight
    if (index != -1 && index == m_highlightedKeyIndex && m_highlightAlpha > 0) {
        QColor highlightColor = Qt::white;
        highlightColor.setAlpha(m_highlightAlpha);
        
        // Blend baseColor with white for the pulse effect
        painter.setPen(QPen(borderColor, 1));
        painter.setBrush(baseColor);
        painter.drawRoundedRect(key.rect, 3, 3);
        
        painter.setBrush(highlightColor);
        painter.drawRoundedRect(key.rect, 3, 3);
    } else {
        painter.setPen(QPen(borderColor, 1));
        painter.setBrush(baseColor);
        painter.drawRoundedRect(key.rect, 3, 3);
    }

    // Add a subtle gradient for a hardware feel
    QLinearGradient grad(key.rect.topLeft(), key.rect.bottomLeft());
    grad.setColorAt(0, QColor(255, 255, 255, 30));
    grad.setColorAt(1, QColor(0, 0, 0, 30));
    painter.setBrush(grad);
    painter.drawRoundedRect(key.rect, 3, 3);

    // Key label
    painter.setPen(textColor);
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    painter.drawText(key.rect, Qt::AlignCenter, key.label);
}

void DlgKeyboardVisualLayout::mousePressEvent(QMouseEvent* event) {
    double scaleX = width() / 900.0;
    double scaleY = height() / 400.0;
    QPointF pos(event->position().x() / scaleX, event->position().y() / scaleY);
    
    for (int i = 0; i < m_keys.size(); ++i) {
        if (m_keys[i].rect.contains(pos)) {
            // Trigger animation
            m_highlightedKeyIndex = i;
            m_highlightAlpha = 180;
            m_animTimer->start();
            
            emit keyClicked(m_keys[i].keySeq);
            
            // Trigger Pulse
            KeyPulse pulse;
            pulse.center = m_keys[i].rect.center();
            pulse.radius = 10;
            pulse.life = 100;
            m_pulses.append(pulse);
            
            // Trigger Popup
            ActionPopup popup;
            popup.pos = m_keys[i].rect.center();
            if (m_seqToConfig.contains(m_keys[i].keySeq)) {
                ConfigKey cfg = m_seqToConfig[m_keys[i].keySeq];
                popup.text = cfg.item;
            } else {
                popup.text = tr("Unmapped");
            }
            popup.life = 100;
            m_popups.append(popup);
            
            update();
            return;
        }
    }
    QDialog::mousePressEvent(event);
}

void DlgKeyboardVisualLayout::slotUpdateAnimation() {
    m_highlightAlpha -= 30;
    if (m_highlightAlpha < 0) m_highlightAlpha = 0;
    
    // Update Pulses
    for (int i = m_pulses.size() - 1; i >= 0; --i) {
        m_pulses[i].radius += 10;
        m_pulses[i].life -= 5;
        if (m_pulses[i].life <= 0) m_pulses.removeAt(i);
    }
    
    // Update Popups
    for (int i = m_popups.size() - 1; i >= 0; --i) {
        m_popups[i].life -= 4;
        if (m_popups[i].life <= 0) m_popups.removeAt(i);
    }

    if (m_highlightAlpha == 0 && m_pulses.isEmpty() && m_popups.isEmpty()) {
        m_highlightedKeyIndex = -1;
        m_animTimer->stop();
    }
    update();
}

void DlgKeyboardVisualLayout::slotKeyPressedFromHardware(const QKeySequence& seq) {
    // Live Echo: Highlight the matching key
    for (int i = 0; i < m_keys.size(); ++i) {
        if (m_keys[i].keySeq == seq) {
            m_highlightedKeyIndex = i;
            m_highlightAlpha = 220; // Brighter for hardware echo
            m_animTimer->start();
            
            // If key learn is active, trigger the mapping editor
            if (m_keyLearnActive) {
                emit keyClicked(seq);
            }
            
            // Trigger Pulse
            KeyPulse pulse;
            pulse.center = m_keys[i].rect.center();
            pulse.radius = 10;
            pulse.life = 100;
            m_pulses.append(pulse);
            
            // Trigger Popup
            ActionPopup popup;
            popup.pos = m_keys[i].rect.center();
            if (m_seqToConfig.contains(seq)) {
                ConfigKey cfg = m_seqToConfig[seq];
                popup.text = cfg.item;
            } else {
                popup.text = tr("Echo");
            }
            popup.life = 100;
            m_popups.append(popup);
            
            break;
        }
    }
    update();
}

void DlgKeyboardVisualLayout::slotToggleKeyLearn(bool enabled) {
    m_keyLearnActive = enabled;
    if (m_pFilter) {
        m_pFilter->setLearningMode(enabled);
    }
    if (enabled) {
        m_pKeyLearnButton->setText(tr("Key Learn ACTIVE - Press Physical Key"));
    } else {
        m_pKeyLearnButton->setText(tr("Enable Key Learn (Hardware Capture)"));
    }
}

void DlgKeyboardVisualLayout::mouseMoveEvent(QMouseEvent* event) {
    double scaleX = width() / 900.0;
    double scaleY = height() / 400.0;
    QPointF pos(event->position().x() / scaleX, event->position().y() / scaleY);
    
    bool found = false;
    for (const KeyInfo& key : m_keys) {
        if (key.rect.contains(pos)) {
            if (m_seqToConfig.contains(key.keySeq)) {
                ConfigKey cfg = m_seqToConfig[key.keySeq];
                QToolTip::showText(event->globalPosition().toPoint(), 
                                 QString("[%1] %2").arg(cfg.group, cfg.item));
            } else {
                QToolTip::showText(event->globalPosition().toPoint(), 
                                 tr("Unmapped: %1").arg(key.keySeq.toString()));
            }
            found = true;
            break;
        }
    }
    if (!found) {
        QToolTip::hideText();
    }
    QDialog::mouseMoveEvent(event);
}

QColor DlgKeyboardVisualLayout::getColorForMapping(const ConfigKey& mapping) {
    if (m_themeIndex == 1) { // Cyberpunk
        if (mapping.group.contains("Channel1")) return QColor(0, 255, 100);
        if (mapping.group.contains("Channel2")) return QColor(0, 200, 255);
        return QColor(100, 255, 150);
    }
    if (m_themeIndex == 2) { // Deep Space
        if (mapping.group.contains("Channel1")) return QColor(255, 50, 50);
        if (mapping.group.contains("Channel2")) return QColor(150, 0, 0);
        return QColor(100, 0, 0);
    }
    if (m_themeIndex == 3) { // Gold
        if (mapping.group.contains("Channel1")) return QColor(255, 215, 0);
        if (mapping.group.contains("Channel2")) return QColor(218, 165, 32);
        return QColor(184, 134, 11);
    }
    
    // Original Neon Blue
    if (mapping.group.contains("Channel1")) return QColor(255, 120, 120); // Soft Red
    if (mapping.group.contains("Channel2")) return QColor(120, 120, 255); // Soft Blue
    if (mapping.group.contains("Channel3")) return QColor(120, 255, 120); // Soft Green
    if (mapping.group.contains("Channel4")) return QColor(255, 255, 120); // Soft Yellow
    if (mapping.group.contains("Library")) return QColor(220, 120, 255); // Soft Purple
    if (mapping.group.contains("Master")) return QColor(255, 180, 100); // Orange
    return QColor(150, 150, 150); // Gray for others
}

void DlgKeyboardVisualLayout::resizeEvent(QResizeEvent* event) {
    QDialog::resizeEvent(event);
}
