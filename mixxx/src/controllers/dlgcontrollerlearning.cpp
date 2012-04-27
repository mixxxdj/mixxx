/**
* @file dlgcontrollerlearning.cpp
* @author Sean M. Pappalardo  spappalardo@mixxx.org
* @date Thu 12 Apr 2012
* @brief The controller mapping learning wizard
*
*/

#include "controllers/dlgcontrollerlearning.h"

DlgControllerLearning::DlgControllerLearning(QWidget * parent,
                                             Controller* controller)
        : QDialog(parent), Ui::DlgControllerLearning(),
          m_pController(controller) {
    qRegisterMetaType<MixxxControl>("MixxxControl");

    setupUi(this);
    labelMappedTo->setText("");
    // Ensure the first page is always shown regardless of the last page shown
    // when the .ui file was saved.
    stackedWidget->setCurrentIndex(0);

    // Delete this dialog when its closed. We don't want any persistence.
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

    connect(pushButtonBegin, SIGNAL(clicked()), this, SLOT(begin()));

    connect(pushButtonNext, SIGNAL(clicked()), this, SLOT(pickControlNext()));
    connect(pushButtonDone, SIGNAL(clicked()), this, SLOT(pickControlDone()));
    connect(m_controlList, SIGNAL(activated(int)), this, SLOT(controlChosen(int)));

    connect(pushButtonBack, SIGNAL(clicked()), this, SLOT(mapControlBack()));
    connect(pushButtonContinue, SIGNAL(clicked()), this, SLOT(mapControlContinue()));

    connect(m_pController, SIGNAL(learnedMessage(QString)), this, SLOT(controlMapped(QString)));
    connect(pushButtonFinish, SIGNAL(clicked()), this, SLOT(close()));
    connect(this, SIGNAL(cancelLearning()), m_pController, SLOT(cancelLearn()));
    connect(this, SIGNAL(learn(MixxxControl)), m_pController, SLOT(learn(MixxxControl)));

    m_deckStr = tr("Deck %1");
    m_samplerStr = tr("Sampler %1");
    m_resetStr = tr("reset to default");

    // Master Controls
    addControl("[Master]", "crossfader", tr("Crossfader"), true);
    addControl("[Master]", "volume", tr("Master volume"), true);
    addControl("[Master]", "balance", tr("Master balance"), true);
    addControl("[Master]", "headVolume", tr("Headphone volume"), true);
    addControl("[Master]", "headMix", tr("Headphone mix (pre/main)"), true);

    // Sampler & Deck Controls -- we combine them to minimize work for
    // translators.

    // Transport
    addDeckControl("playposition", tr("Strip-search through track"));
    addDeckAndSamplerControl("play", tr("Play button"));
    addDeckControl("back", tr("Fast rewind button"));
    addDeckControl("start", tr("Jump to start of track"));
    addDeckControl("end", tr("Jump to end of track"));
    addDeckControl("fwd", tr("Fast forward button"));
    addDeckControl("reverse", tr("Play reverse button"));

    // Modes and Mixing
    addDeckControl("volume", tr("Volume fader"), true);
    addDeckAndSamplerControl("pregain", tr("Gain knob"), true);
    addDeckAndSamplerControl("pfl", tr("Headphone listen button"));
    addDeckAndSamplerControl("keylock", tr("Toggle keylock mode"));
    addDeckControl("quantize", tr("Toggle quantize mode"));
    addDeckAndSamplerControl("repeat", tr("Toggle repeat mode"));
    addDeckAndSamplerControl("eject", tr("Eject track"));
    addDeckControl("beats_translate_curpos", tr("Adjust beatgrid"));
    addSamplerControl("orientation", tr("Mix orientation (e.g. left, right, center)"));

    // BPM & Sync
    addDeckAndSamplerControl("bpm_tap", tr("BPM tap button"));
    addDeckControl("beatsync", tr("Beat sync (tempo and phase)"));
    addDeckControl("beatsync_tempo", tr("Beat sync (tempo only)"));
    addDeckControl("beatsync_phase", tr("Beat sync (phase only)"));

    // Rate
    addDeckAndSamplerControl("rate", tr("Pitch control slider"), true);
    addDeckControl("rate_perm_up", tr("Adjust rate up (course)"));
    addDeckControl("rate_perm_up_small", tr("Adjust rate up (fine)"));
    addDeckControl("rate_perm_down", tr("Adjust rate down (course)"));
    addDeckControl("rate_perm_down_small", tr("Adjust rate down (fine)"));
    addDeckControl("rate_temp_up", tr("Pitch-bend rate up (course)"));
    addDeckControl("rate_temp_up_small", tr("Pitch-bend rate up (fine)"));
    addDeckControl("rate_temp_down", tr("Pitch-bend rate down (course)"));
    addDeckControl("rate_temp_down_small", tr("Pitch-bend rate down (fine)"));

    // EQs
    addDeckControl("filterHigh", tr("High EQ knob"), true);
    addDeckControl("filterMid", tr("Mid EQ knob"), true);
    addDeckControl("filterLow", tr("Low EQ knob"), true);
    addDeckControl("filterHighKill", tr("High EQ kill"));
    addDeckControl("filterMidKill", tr("Mid EQ kill"));
    addDeckControl("filterLowKill", tr("Low EQ kill"));

    // Vinyl Control
    addDeckControl("vinylcontrol_cueing", tr("Toggle vinyl-control cueing mode (OFF/ONE/HOT)"));
    addDeckControl("vinylcontrol_mode", tr("Toggle vinyl-control mode (ABS/REL/CONST)"));

    // Cues
    addDeckControl("cue_default", tr("Cue button"));
    addDeckControl("cue_set", tr("Set cue point"));
    addDeckControl("cue_gotoandstop", tr("Go to cue point and stop"));

    // Hotcues
    QString hotcueActivate = tr("Set or jump to hotcue %1");
    QString hotcueClear = tr("Clear hotcue %1");
    QString hotcueGoto = tr("Jump to hotcue %1");
    QString hotcueGotoAndStop = tr("Jump to hotcue %1 and stop");
    for (int i = 1; i <= 4; ++i) {
        addDeckAndSamplerControl(QString("hotcue_%1_activate").arg(i),
                       hotcueActivate.arg(QString::number(i)));
        addDeckAndSamplerControl(QString("hotcue_%1_clear").arg(i),
                       hotcueClear.arg(QString::number(i)));
        addDeckAndSamplerControl(QString("hotcue_%1_goto").arg(i),
                       hotcueGoto.arg(QString::number(i)));
        addDeckAndSamplerControl(QString("hotcue_%1_gotoandstop").arg(i),
                       hotcueGotoAndStop.arg(QString::number(i)));
    }

    // Loops
    addDeckControl("loop_in", tr("Loop In button"));
    addDeckControl("loop_out", tr("Loop Out button"));
    addDeckControl("reloop_exit", tr("Reloop / Exit button"));
    addDeckControl("loop_halve", tr("Halve the current loop's length"));
    addDeckControl("loop_double", tr("Double the current loop's length"));

    // Beatloops
    QString beatLoop = tr("Create %1-beat loop");
    addDeckControl("beatloop_0.0625",  beatLoop.arg("1/16th"));
    addDeckControl("beatloop_0.125", beatLoop.arg("1/8th"));
    addDeckControl("beatloop_0.25", beatLoop.arg("1/4th"));
    addDeckControl("beatloop_0.5", beatLoop.arg("1/2"));
    addDeckControl("beatloop_1", beatLoop.arg("1"));
    addDeckControl("beatloop_2", beatLoop.arg("2"));
    addDeckControl("beatloop_4", beatLoop.arg("4"));
    addDeckControl("beatloop_8", beatLoop.arg("8"));
    addDeckControl("beatloop_16", beatLoop.arg("16"));

    ControlObject *co = ControlObject::getControl(ConfigKey("[Master]", "num_decks"));
    const int iNumDecks = co->get();
    QString spinnyText = tr("Show/hide spinning vinyl widget");
    for (int i = 1; i <= iNumDecks; ++i) {
        addControl(QString("[Spinny%1]").arg(i), "show_spinny",
                   QString("%1: %2").arg(m_deckStr.arg(i), spinnyText));
    }


    // Library Controls
    addControl("[Playlist]", "SelectNextPlaylist", tr("Switch to the next view (library, playlist..)"));
    addControl("[Playlist]", "SelectPrevPlaylist", tr("Switch to the previous view (library, playlist..)"));
    addControl("[Playlist]", "SelectNextTrack", tr("Scroll to next track in library/playlist"));
    addControl("[Playlist]", "SelectPrevTrack", tr("Scroll to previous track in library/playlist"));
    addControl("[Playlist]", "LoadSelectedIntoFirstStopped", tr("Load selected track into first stopped player"));
    addDeckAndSamplerControl("LoadSelectedTrack", tr("Load selected track"));

    // Flanger Controls
    addDeckControl("flanger", tr("Toggle flange effect"));
    addControl("[Flanger]", "lfoPeriod", tr("Flange effect: Wavelength/period"), true);
    addControl("[Flanger]", "lfoDepth", tr("Flange effect: Intensity"), true);
    addControl("[Flanger]", "lfoDelay", tr("Flange effect: Phase delay"), true);

    // Microphone Controls
    addControl("[Microphone]", "talkover", tr("Microphone on/off"));
    addControl("[Microphone]", "volume", tr("Microphone volume"), true);
    addControl("[Microphone]", "orientation", tr("Microphone channel orientation (e.g. left, right, center)"));

    // Skin Controls
    addControl("[Samplers]", "show_samplers", tr("Show/hide the sampler section"));
    addControl("[Microphone]", "show_microphone", tr("Show/hide the microphone section"));
    addControl("[Vinylcontrol]", "show_vinylcontrol", tr("Show/hide the vinyl control section"));

    // Sort them by group so you work with one deck at a time
    //  This also puts them in alphabetical order by item
    qSort(m_controlsAvailable);

    // TODO: Sort them by group in the order they were entered
    //qSort(m_controlsAvailable.begin(), m_controlsAvailable.end(), groupLessThan);
}

DlgControllerLearning::~DlgControllerLearning() {
    //If there was any ongoing learning, cancel it (benign if there wasn't).
    emit(cancelLearning());
}

void DlgControllerLearning::addControl(QString group, QString control, QString description,
                                       bool addReset) {
    m_controlsAvailable.append(MixxxControl(group, control, description));
    if (addReset) {
        QString resetDescription = QString("%1 (%2)").arg(description, m_resetStr);
        QString resetControl = QString("%1_set_default").arg(control);
        m_controlsAvailable.append(MixxxControl(group, resetControl, resetDescription));
    }
}

void DlgControllerLearning::addDeckAndSamplerControl(QString control, QString controlDescription,
                                                     bool addReset) {
    addDeckControl(control, controlDescription, addReset);
    addSamplerControl(control, controlDescription, addReset);
}

void DlgControllerLearning::addDeckControl(QString control, QString controlDescription,
                                           bool addReset) {
    ControlObject *co = ControlObject::getControl(ConfigKey("[Master]", "num_decks"));
    const int iNumDecks = co->get();
    for (int i = 1; i <= iNumDecks; ++i) {
        QString group = QString("[Channel%1]").arg(i);

        QString description = QString("%1: %2").arg(
            m_deckStr.arg(QString::number(i)), controlDescription);
        m_controlsAvailable.append(MixxxControl(group, control, description));

        if (addReset) {
            QString resetDescription = QString("%1 (%2)").arg(description, m_resetStr);
            QString resetControl = QString("%1_set_default").arg(control);
            m_controlsAvailable.append(MixxxControl(group, resetControl, resetDescription));
        }
    }
}

void DlgControllerLearning::addSamplerControl(QString control, QString controlDescription,
                                              bool addReset) {
    ControlObject *co = ControlObject::getControl(ConfigKey("[Master]", "num_samplers"));
    const int iNumSamplers = co->get();
    for (int i = 1; i <= iNumSamplers; ++i) {
        QString group = QString("[Sampler%1]").arg(i);
        QString description = QString("%1: %2").arg(
            m_samplerStr.arg(QString::number(i)), controlDescription);
        m_controlsAvailable.append(MixxxControl(group, control, description));
        if (addReset) {
            QString resetDescription = QString("%1 (%2)").arg(description, m_resetStr);
            QString resetControl = QString("%1_set_default").arg(control);
            m_controlsAvailable.append(MixxxControl(group, resetControl, resetDescription));
        }
    }
}

void DlgControllerLearning::begin() {
    // Switch pages in the stacked widget so that we show the choose-a-control
    // page.
    showPickControl();
}

void DlgControllerLearning::pickControlNext() {
    emit(stopListeningForClicks());
    showMapControl();
}

void DlgControllerLearning::pickControlDone() {
    emit(stopListeningForClicks());
    stackedWidget->setCurrentIndex(3);
}

void DlgControllerLearning::showPickControl() {
    emit(listenForClicks());
    populateComboBox();
    pushButtonNext->setEnabled(false);
    controlToMapMessage->setText("");
    stackedWidget->setCurrentIndex(1);
    m_currentControl = MixxxControl();
}

void DlgControllerLearning::loadControl(const MixxxControl& control) {
    m_currentControl = control;
    QString message = tr("Ready to map control: %1.\n Click next to continue.").arg(m_currentControl.description());
    controlToMapMessage->setText(message);
    pushButtonNext->setEnabled(true);
}

void DlgControllerLearning::controlChosen(int controlIndex) {
    if (controlIndex < 0 || controlIndex >= m_controlsAvailable.size()) {
        return;
    }
    loadControl(m_controlsAvailable[controlIndex]);
}

void DlgControllerLearning::controlClicked(ControlObject* pControl) {
    if (!pControl) {
        return;
    }
    ConfigKey key = pControl->getKey();
    // Lookup MixxxControl
    for (int i = 0; i < m_controlsAvailable.size(); ++i) {
        const MixxxControl& control = m_controlsAvailable[i];
        if (control.group() == key.group && control.item() == key.item) {
            m_controlList->setCurrentIndex(i);
            loadControl(control);
            return;
        }
    }
}


void DlgControllerLearning::showMapControl() {
    emit(learn(m_currentControl));
    labelMixxxControl->setText(m_currentControl.description());
    pushButtonContinue->setEnabled(false);
    pushButtonBack->setEnabled(true);
    labelMappedTo->setText("");
    stackedWidget->setCurrentIndex(2);
}

void DlgControllerLearning::controlMapped(QString message) {
    labelMappedTo->setText(tr("Successfully mapped to: ") + message);
    pushButtonContinue->setEnabled(true);
    pushButtonBack->setEnabled(false);
}

void DlgControllerLearning::mapControlBack() {
    // TODO(XXX) Make sure this doesn't break anything.
    emit(learn(MixxxControl()));
    showPickControl();
}

void DlgControllerLearning::mapControlContinue() {
    // Save the mapping? Currently the controller saves it itself.
    showPickControl();
}

void DlgControllerLearning::populateComboBox() {
    m_controlList->clear();
    foreach (MixxxControl control, m_controlsAvailable) {
        QString text = control.description();
        if (m_mappedControls.contains(control)) {
            text = QString("<b>%1</b> (already mapped)").arg(text);
        }
        m_controlList->addItem(text);
    }
}
