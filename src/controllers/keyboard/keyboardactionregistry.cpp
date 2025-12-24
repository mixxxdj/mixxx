#include "controllers/keyboard/keyboardactionregistry.h"

#include <QtDebug>

#include "moc_keyboardactionregistry.cpp"

KeyboardActionRegistry::KeyboardActionRegistry(QObject* parent)
        : QObject(parent) {
    buildRegistry();
}

KeyboardActionRegistry::~KeyboardActionRegistry() {
}

void KeyboardActionRegistry::buildRegistry() {
    m_actions.clear();
    m_categorizedActions.clear();

    // Register controls for decks
    registerDeckControls("[Channel1]");
    registerDeckControls("[Channel2]");
    registerDeckControls("[Channel3]");
    registerDeckControls("[Channel4]");

    // Register preview deck
    registerDeckControls("[PreviewDeck1]");

    // Register master controls
    registerMasterControls();

    // Register library controls
    registerLibraryControls();

    // Register effect controls
    registerEffectControls();

    // Register sampler controls
    for (int i = 1; i <= 64; i++) {
        registerSamplerControls();
    }
}

void KeyboardActionRegistry::registerAction(
        const ConfigKey& key,
        const QString& category,
        const QString& displayName,
        const QString& description) {
    ControlActionInfo info(key, category, displayName, description);
    m_actions[key] = info;
    m_categorizedActions[category].append(key);
}

void KeyboardActionRegistry::registerDeckControls(const QString& group) {
    QString deckName = group;
    if (group == "[Channel1]") deckName = "Deck 1";
    else if (group == "[Channel2]") deckName = "Deck 2";
    else if (group == "[Channel3]") deckName = "Deck 3";
    else if (group == "[Channel4]") deckName = "Deck 4";
    else if (group == "[PreviewDeck1]") deckName = "Preview Deck";

    QString category = "Playback";

    // Playback controls
    registerAction(ConfigKey(group, "play"), category,
                  QString("%1: Play/Pause").arg(deckName),
                  "Toggle playback");
    registerAction(ConfigKey(group, "cue_default"), category,
                  QString("%1: Cue").arg(deckName),
                  "Cue point (press and hold)");
    registerAction(ConfigKey(group, "cue_set"), category,
                  QString("%1: Set Cue").arg(deckName),
                  "Set cue point at current position");
    registerAction(ConfigKey(group, "cue_gotoandstop"), category,
                  QString("%1: Go to Cue and Stop").arg(deckName),
                  "Jump to cue point and stop");
    registerAction(ConfigKey(group, "start_stop"), category,
                  QString("%1: Start/Stop").arg(deckName),
                  "Start or stop playback");

    // Transport controls
    registerAction(ConfigKey(group, "back"), category,
                  QString("%1: Rewind").arg(deckName),
                  "Fast rewind");
    registerAction(ConfigKey(group, "fwd"), category,
                  QString("%1: Fast Forward").arg(deckName),
                  "Fast forward");
    registerAction(ConfigKey(group, "reverse"), category,
                  QString("%1: Reverse").arg(deckName),
                  "Play in reverse");

    // Sync controls
    category = "Sync & Tempo";
    registerAction(ConfigKey(group, "sync_enabled"), category,
                  QString("%1: Sync").arg(deckName),
                  "Enable sync");
    registerAction(ConfigKey(group, "bpm_tap"), category,
                  QString("%1: Tap BPM").arg(deckName),
                  "Tap to set BPM");
    registerAction(ConfigKey(group, "beatsync"), category,
                  QString("%1: Beat Sync").arg(deckName),
                  "Sync to beat");

    // Pitch/Rate controls
    registerAction(ConfigKey(group, "rate_perm_up"), category,
                  QString("%1: Pitch Up").arg(deckName),
                  "Increase pitch permanently");
    registerAction(ConfigKey(group, "rate_perm_down"), category,
                  QString("%1: Pitch Down").arg(deckName),
                  "Decrease pitch permanently");
    registerAction(ConfigKey(group, "rate_perm_up_small"), category,
                  QString("%1: Pitch Up (Small)").arg(deckName),
                  "Increase pitch slightly");
    registerAction(ConfigKey(group, "rate_perm_down_small"), category,
                  QString("%1: Pitch Down (Small)").arg(deckName),
                  "Decrease pitch slightly");
    registerAction(ConfigKey(group, "rate_temp_up"), category,
                  QString("%1: Pitch Bend Up").arg(deckName),
                  "Temporary pitch increase (nudge)");
    registerAction(ConfigKey(group, "rate_temp_down"), category,
                  QString("%1: Pitch Bend Down").arg(deckName),
                  "Temporary pitch decrease (nudge)");
    registerAction(ConfigKey(group, "rate_temp_up_small"), category,
                  QString("%1: Pitch Bend Up (Small)").arg(deckName),
                  "Small temporary pitch increase");
    registerAction(ConfigKey(group, "rate_temp_down_small"), category,
                  QString("%1: Pitch Bend Down (Small)").arg(deckName),
                  "Small temporary pitch decrease");

    // Loop controls
    category = "Looping";
    registerAction(ConfigKey(group, "loop_in"), category,
                  QString("%1: Loop In").arg(deckName),
                  "Set loop in point");
    registerAction(ConfigKey(group, "loop_out"), category,
                  QString("%1: Loop Out").arg(deckName),
                  "Set loop out point");
    registerAction(ConfigKey(group, "loop_in_goto"), category,
                  QString("%1: Go to Loop In").arg(deckName),
                  "Jump to loop in point");
    registerAction(ConfigKey(group, "loop_out_goto"), category,
                  QString("%1: Go to Loop Out").arg(deckName),
                  "Jump to loop out point");
    registerAction(ConfigKey(group, "reloop_toggle"), category,
                  QString("%1: Reloop/Exit").arg(deckName),
                  "Toggle loop on/off");
    registerAction(ConfigKey(group, "reloop_andstop"), category,
                  QString("%1: Reloop and Stop").arg(deckName),
                  "Enable loop and stop playback");
    registerAction(ConfigKey(group, "loop_halve"), category,
                  QString("%1: Loop Halve").arg(deckName),
                  "Halve loop size");
    registerAction(ConfigKey(group, "loop_double"), category,
                  QString("%1: Loop Double").arg(deckName),
                  "Double loop size");
    registerAction(ConfigKey(group, "beatloop_activate"), category,
                  QString("%1: Beatloop").arg(deckName),
                  "Activate beatloop");
    registerAction(ConfigKey(group, "beatlooproll_activate"), category,
                  QString("%1: Beatloop Roll").arg(deckName),
                  "Activate beatloop roll");

    // Beat jump controls
    category = "Beat Jump";
    registerAction(ConfigKey(group, "beatjump_forward"), category,
                  QString("%1: Beat Jump Forward").arg(deckName),
                  "Jump forward by beats");
    registerAction(ConfigKey(group, "beatjump_backward"), category,
                  QString("%1: Beat Jump Backward").arg(deckName),
                  "Jump backward by beats");
    registerAction(ConfigKey(group, "beatjump_1_forward"), category,
                  QString("%1: Beat Jump +1").arg(deckName),
                  "Jump forward 1 beat");
    registerAction(ConfigKey(group, "beatjump_1_backward"), category,
                  QString("%1: Beat Jump -1").arg(deckName),
                  "Jump backward 1 beat");

    // Hotcue controls
    category = "Hotcues";
    for (int i = 1; i <= 8; i++) {
        registerAction(ConfigKey(group, QString("hotcue_%1_activate").arg(i)), category,
                      QString("%1: Hotcue %2 Activate").arg(deckName).arg(i),
                      QString("Activate hotcue %1").arg(i));
        registerAction(ConfigKey(group, QString("hotcue_%1_clear").arg(i)), category,
                      QString("%1: Hotcue %2 Clear").arg(deckName).arg(i),
                      QString("Clear hotcue %1").arg(i));
        registerAction(ConfigKey(group, QString("hotcue_%1_set").arg(i)), category,
                      QString("%1: Hotcue %2 Set").arg(deckName).arg(i),
                      QString("Set hotcue %1").arg(i));
    }

    // Track loading
    category = "Track Loading";
    registerAction(ConfigKey(group, "LoadSelectedTrack"), category,
                  QString("%1: Load Selected Track").arg(deckName),
                  "Load the selected track from library");
    registerAction(ConfigKey(group, "LoadSelectedTrackAndPlay"), category,
                  QString("%1: Load and Play").arg(deckName),
                  "Load selected track and start playing");
    registerAction(ConfigKey(group, "eject"), category,
                  QString("%1: Eject").arg(deckName),
                  "Eject/unload track");

    // Monitoring
    category = "Monitoring";
    registerAction(ConfigKey(group, "pfl"), category,
                  QString("%1: Headphone Cue (PFL)").arg(deckName),
                  "Toggle pre-fader listen");

    // Vinyl control
    category = "Vinyl Control";
    registerAction(ConfigKey(group, "passthrough"), category,
                  QString("%1: Passthrough").arg(deckName),
                  "Enable passthrough mode");
    registerAction(ConfigKey(group, "vinylcontrol_mode"), category,
                  QString("%1: Vinyl Control Mode").arg(deckName),
                  "Toggle vinyl control mode");
    registerAction(ConfigKey(group, "vinylcontrol_cueing"), category,
                  QString("%1: Vinyl Control Cueing").arg(deckName),
                  "Toggle vinyl control cueing");

    // EQ controls
    category = "EQ";
    QString eqGroup = QString("[EqualizerRack1_%1_Effect1]").arg(group);
    registerAction(ConfigKey(eqGroup, "button_parameter1"), category,
                  QString("%1: Kill Low").arg(deckName),
                  "Kill low frequencies");
    registerAction(ConfigKey(eqGroup, "button_parameter2"), category,
                  QString("%1: Kill Mid").arg(deckName),
                  "Kill mid frequencies");
    registerAction(ConfigKey(eqGroup, "button_parameter3"), category,
                  QString("%1: Kill High").arg(deckName),
                  "Kill high frequencies");
    registerAction(ConfigKey(eqGroup, "parameter1"), category,
                  QString("%1: Low EQ").arg(deckName),
                  "Adjust low frequencies");
    registerAction(ConfigKey(eqGroup, "parameter2"), category,
                  QString("%1: Mid EQ").arg(deckName),
                  "Adjust mid frequencies");
    registerAction(ConfigKey(eqGroup, "parameter3"), category,
                  QString("%1: High EQ").arg(deckName),
                  "Adjust high frequencies");

    // Filter
    category = "Filter";
    QString filterGroup = QString("[QuickEffectRack1_%1_Effect1]").arg(group);
    registerAction(ConfigKey(filterGroup, "parameter1"), category,
                  QString("%1: Quick Filter").arg(deckName),
                  "Adjust quick effect filter (usually LPF/HPF)");

    // Volume & Gain
    category = "Mixer";
    registerAction(ConfigKey(group, "volume"), category,
                  QString("%1: Volume").arg(deckName),
                  "Adjust deck volume");
    registerAction(ConfigKey(group, "pregain"), category,
                  QString("%1: Gain").arg(deckName),
                  "Adjust track gain");

    // Slip mode & Quantize
    category = "Transport";
    registerAction(ConfigKey(group, "slip_enabled"), category,
                  QString("%1: Slip Mode").arg(deckName),
                  "Toggle slip mode");
    registerAction(ConfigKey(group, "quantize"), category,
                  QString("%1: Quantize").arg(deckName),
                  "Toggle quantize");
    registerAction(ConfigKey(group, "keylock"), category,
                  QString("%1: Keylock").arg(deckName),
                  "Toggle keylock (Master Tempo)");
}

void KeyboardActionRegistry::registerMasterControls() {
    QString category = "Master";

    registerAction(ConfigKey("[Master]", "crossfader"), category,
                  "Crossfader",
                  "Adjust crossfader position");
    registerAction(ConfigKey("[Master]", "crossfader_up"), category,
                  "Crossfader Right",
                  "Move crossfader to the right");
    registerAction(ConfigKey("[Master]", "crossfader_down"), category,
                  "Crossfader Left",
                  "Move crossfader to the left");
    registerAction(ConfigKey("[Master]", "crossfader_up_small"), category,
                  "Crossfader Right (Small)",
                  "Move crossfader slightly right");
    registerAction(ConfigKey("[Master]", "crossfader_down_small"), category,
                  "Crossfader Left (Small)",
                  "Move crossfader slightly left");
    registerAction(ConfigKey("[Master]", "volume"), category,
                  "Master Volume",
                  "Adjust master output volume");
    registerAction(ConfigKey("[Master]", "balance"), category,
                  "Master Balance",
                  "Adjust master balance");

    // Microphone
    category = "Microphone";
    registerAction(ConfigKey("[Microphone]", "talkover"), category,
                  "Microphone Talkover",
                  "Enable microphone talkover");
    registerAction(ConfigKey("[Microphone]", "volume"), category,
                  "Microphone Volume",
                  "Adjust microphone volume");

    // View & Layout
    category = "View";
    registerAction(ConfigKey("[Master]", "maximize_library"), category,
                  "Maximize Library",
                  "Toggle maximized library view");
    registerAction(ConfigKey("[Keyboard]", "cycle_focus_deck"), category,
                  "Cycle Focused Deck",
                  "Switch between active decks for keyboard control");
}

void KeyboardActionRegistry::registerLibraryControls() {
    QString category = "Library";

    registerAction(ConfigKey("[Library]", "EditItem"), category,
                  "Edit Track Metadata",
                  "Edit the selected track's metadata");
    registerAction(ConfigKey("[Library]", "MoveUp"), category,
                  "Move Selection Up",
                  "Move selection up in library");
    registerAction(ConfigKey("[Library]", "MoveDown"), category,
                  "Move Selection Down",
                  "Move selection down in library");
    registerAction(ConfigKey("[Library]", "MoveVertical"), category,
                  "Move Selection Vertical",
                  "Move selection vertically");
    registerAction(ConfigKey("[Library]", "ScrollUp"), category,
                  "Scroll Up",
                  "Scroll library up");
    registerAction(ConfigKey("[Library]", "ScrollDown"), category,
                  "Scroll Down",
                  "Scroll library down");
    registerAction(ConfigKey("[Skin]", "show_maximized_library"), category,
                  "Maximize Library",
                  "Toggle maximized library view");
}

void KeyboardActionRegistry::registerEffectControls() {
    // Effect racks are complex, register basic ones
    QString category = "Effects";

    for (int rack = 1; rack <= 4; rack++) {
        for (int unit = 1; unit <= 4; unit++) {
            QString group = QString("[EffectRack%1_EffectUnit%2]").arg(rack).arg(unit);
            registerAction(ConfigKey(group, "enabled"), category,
                          QString("Effect Rack %1 Unit %2 Enable").arg(rack).arg(unit),
                          "Enable/disable effect unit");
        }
    }
}

void KeyboardActionRegistry::registerSamplerControls() {
    QString category = "Samplers";

    for (int i = 1; i <= 64; i++) {
        QString group = QString("[Sampler%1]").arg(i);
        registerAction(ConfigKey(group, "cue_gotoandplay"), category,
                      QString("Sampler %1: Play").arg(i),
                      QString("Play sampler %1").arg(i));
        registerAction(ConfigKey(group, "cue_gotoandstop"), category,
                      QString("Sampler %1: Stop").arg(i),
                      QString("Stop sampler %1").arg(i));
    }
}

QList<ControlActionInfo> KeyboardActionRegistry::getAllActions() const {
    return m_actions.values();
}

QList<ControlActionInfo> KeyboardActionRegistry::getActionsByCategory(const QString& category) const {
    QList<ControlActionInfo> result;
    QList<ConfigKey> keys = m_categorizedActions.value(category);
    for (const ConfigKey& key : keys) {
        result.append(m_actions.value(key));
    }
    return result;
}

QStringList KeyboardActionRegistry::getCategories() const {
    QStringList categories = m_categorizedActions.keys();
    categories.sort();
    return categories;
}

QList<ControlActionInfo> KeyboardActionRegistry::searchActions(const QString& query) const {
    QList<ControlActionInfo> result;
    QString lowerQuery = query.toLower();

    for (const ControlActionInfo& info : m_actions.values()) {
        if (info.displayName.toLower().contains(lowerQuery) ||
            info.description.toLower().contains(lowerQuery) ||
            info.key.group.toLower().contains(lowerQuery) ||
            info.key.item.toLower().contains(lowerQuery)) {
            result.append(info);
        }
    }

    return result;
}

ControlActionInfo KeyboardActionRegistry::getActionInfo(const ConfigKey& key) const {
    return m_actions.value(key);
}

bool KeyboardActionRegistry::hasAction(const ConfigKey& key) const {
    return m_actions.contains(key);
}
