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
    iCurrentControl = 0;
    // Ensure the first page is always shown regardless of the last page shown
    // when the .ui file was saved.
    stackedWidget->setCurrentIndex(0);

    // Delete this dialog when its closed. We don't want any persistence.
    QWidget::setAttribute(Qt::WA_DeleteOnClose);

    m_pSkipShortcut = new QShortcut(QKeySequence(Qt::Key_Space), this);

    connect(pushButtonBegin, SIGNAL(clicked()), this, SLOT(begin()));
    connect(pushButtonSkip, SIGNAL(clicked()), this, SLOT(next()));
    connect(pushButtonPrevious, SIGNAL(clicked()), this, SLOT(prev()));
    connect(m_pController, SIGNAL(learnedMessage(QString)), this, SLOT(controlMapped(QString)));
    connect(pushButtonFinish, SIGNAL(clicked()), this, SLOT(close()));
    connect(this, SIGNAL(cancelLearning()), m_pController, SLOT(cancelLearn()));
    connect(this, SIGNAL(learn(MixxxControl)), m_pController, SLOT(learn(MixxxControl)));


    // Master Controls
    addControl("[Master]", "crossfader", tr("Crossfader"));
    addControl("[Master]", "volume", tr("Master volume"));
    addControl("[Master]", "balance", tr("Master balance"));
    addControl("[Master]", "headVolume", tr("Headphone volume"));
    addControl("[Master]", "headMix", tr("Headphone mix (pre/main)"));

    // Deck Controls
    addDeckControl("cue_default", tr("Deck %1: Cue button"));
    addDeckControl("play", tr("Deck %1: Play button"));
    addDeckControl("back", tr("Deck %1: Fast rewind button"));
    addDeckControl("fwd", tr("Deck %1: Fast forward button"));
    addDeckControl("reverse", tr("Deck %1: Play reverse button"));
    addDeckControl("pfl", tr("Deck %1: Headphone listen button"));
    addDeckControl("beatsync", tr("Deck %1: Beat sync button"));
    addDeckControl("bpm", tr("Deck %1: BPM tap button"));
    addDeckControl("keylock", tr("Deck %1: Keylock button"));
    addDeckControl("rate", tr("Deck %1: Pitch control slider"));
    addDeckControl("flanger", tr("Deck %1: Flanger effect button"));
    addDeckControl("volume", tr("Deck %1: Volume fader"));
    addDeckControl("pregain", tr("Deck %1: Gain knob"));
    addDeckControl("filterHigh", tr("Deck %1: High EQ knob"));
    addDeckControl("filterMid", tr("Deck %1: Mid EQ knob"));
    addDeckControl("filterLow", tr("Deck %1: Low EQ knob"));
    addDeckControl("loop_in", tr("Deck %1: Loop In button"));
    addDeckControl("loop_out", tr("Deck %1: Loop Out button"));
    addDeckControl("reloop_exit", tr("Deck %1: Reloop / Exit button"));
    addDeckControl("beatloop_4", tr("Deck %1: Set up a loop over 4 beats"));
    addDeckControl("loop_halve", tr("Deck %1: Halve the current loop's length"));
    addDeckControl("loop_double", tr("Deck %1: Double the current loop's length"));
    addDeckControl("hotcue_1_activate", tr("Deck %1: Hotcue 1 button"));
    addDeckControl("hotcue_2_activate", tr("Deck %1: Hotcue 2 button"));
    addDeckControl("hotcue_3_activate", tr("Deck %1: Hotcue 3 button"));
    addDeckControl("hotcue_4_activate", tr("Deck %1: Hotcue 4 button"));
    addDeckControl("hotcue_1_clear", tr("Deck %1: Hotcue 1 delete button"));
    addDeckControl("hotcue_2_clear", tr("Deck %1: Hotcue 2 delete button"));
    addDeckControl("hotcue_3_clear", tr("Deck %1: Hotcue 3 delete button"));
    addDeckControl("hotcue_4_clear", tr("Deck %1: Hotcue 4 delete button"));

    addSamplerControl("play", tr("Sampler %1: Play button"));
    addSamplerControl("pregain", tr("Sampler %1: Gain knob"));
    addSamplerControl("pfl", tr("Sampler %1: Headphone listen button"));
    addSamplerControl("bpm", tr("Sampler %1: BPM tap button"));
    addSamplerControl("keylock", tr("Sampler %1: Keylock button"));
    addSamplerControl("rate", tr("Sampler %1: Pitch control slider"));
    addSamplerControl("hotcue_1_activate", tr("Sampler %1: Hotcue 1 button"));
    addSamplerControl("hotcue_2_activate", tr("Sampler %1: Hotcue 2 button"));
    addSamplerControl("hotcue_3_activate", tr("Sampler %1: Hotcue 3 button"));
    addSamplerControl("hotcue_4_activate", tr("Sampler %1: Hotcue 4 button"));
    addSamplerControl("hotcue_1_clear", tr("Sampler %1: Hotcue 1 delete button"));
    addSamplerControl("hotcue_2_clear", tr("Sampler %1: Hotcue 2 delete button"));
    addSamplerControl("hotcue_3_clear", tr("Sampler %1: Hotcue 3 delete button"));
    addSamplerControl("hotcue_4_clear", tr("Sampler %1: Hotcue 4 delete button"));

    // Library Controls
    addControl("[Playlist]", "SelectNextPlaylist", tr("Switch to the next view (library, playlist..)"));
    addControl("[Playlist]", "SelectPrevPlaylist", tr("Switch to the previous view (library, playlist..)"));
    addControl("[Playlist]", "SelectNextTrack", tr("Scroll to next track in library/playlist"));
    addControl("[Playlist]", "SelectPrevTrack", tr("Scroll to previous track in library/playlist"));
    addControl("[Playlist]", "LoadSelectedIntoFirstStopped", tr("Load selected track into first stopped player"));
    addDeckControl("LoadSelectedTrack", tr("Deck %1: Load selected track"));

    // Flanger Controls
    addControl("[Flanger]", "lfoPeriod", tr("Flange effect: Wavelength/period"));
    addControl("[Flanger]", "lfoDepth", tr("Flange effect: Intensity"));
    addControl("[Flanger]", "lfoDelay", tr("Flange effect: Phase delay"));

    // Microphone Controls
    addControl("[Microphone]", "talkover", tr("Microphone on/off"));
    addControl("[Microphone]", "volume", tr("Microphone volume"));

    // Sort them by group so you work with one deck at a time
    //  This also puts them in alphabetical order by item
    qSort(m_controlsToMap);

    // TODO: Sort them by group in the order they were entered
    //qSort(m_controlsToMap.begin(), m_controlsToMap.end(), groupLessThan);
}

DlgControllerLearning::~DlgControllerLearning() {
    //If there was any ongoing learning, cancel it (benign if there wasn't).
    emit(cancelLearning());

    delete m_pSkipShortcut;
}

void DlgControllerLearning::addControl(QString group, QString control, QString description) {
    m_controlsToMap.append(MixxxControl(group, control, description));
}

void DlgControllerLearning::addDeckControl(QString control, QString description) {
    ControlObject *co = ControlObject::getControl(ConfigKey("[Master]", "num_decks"));
    const int iNumDecks = co->get();
    for (int i = 1; i <= iNumDecks; ++i) {
        QString group = QString("[Channel%1]").arg(i);
        m_controlsToMap.append(MixxxControl(group, control, description.arg(i)));
    }
}

void DlgControllerLearning::addSamplerControl(QString control, QString description) {
    ControlObject *co = ControlObject::getControl(ConfigKey("[Master]", "num_samplers"));
    const int iNumSamplers = co->get();
    for (int i = 1; i <= iNumSamplers; ++i) {
        QString group = QString("[Sampler%1]").arg(i);
        m_controlsToMap.append(MixxxControl(group, control, description.arg(i)));
    }
}

void DlgControllerLearning::begin() {
    // Switch pages in the stacked widget so that we show the mapping stuff.
    stackedWidget->setCurrentIndex(1);

    connect(m_pSkipShortcut, SIGNAL(activated()), pushButtonSkip, SLOT(click()));

    //Tell the MIDI mapping to start learning the first control.
    iCurrentControl = 0;
    MixxxControl control = m_controlsToMap[iCurrentControl];
    emit(learn(control));
    QString count = QString(tr("(Control %1 of %2)")).arg(iCurrentControl+1)
                        .arg(m_controlsToMap.size()+1);
    labelMixxxControl->setText(control.description()+"\n"+count);
}

void DlgControllerLearning::next() {
    iCurrentControl++;
    if (iCurrentControl < m_controlsToMap.size())
    {
        MixxxControl control = m_controlsToMap[iCurrentControl];
        emit(learn(control));
        QString count = QString(tr("(Control %1 of %2)")).arg(iCurrentControl+1)
                        .arg(m_controlsToMap.size()+1);
        labelMixxxControl->setText(control.description()+"\n"+count);
        pushButtonSkip->setText(tr("Skip"));

        labelMappedTo->setText("");
        pushButtonPrevious->setEnabled(true);
    }
    else
    {
        disconnect(m_pSkipShortcut, SIGNAL(activated()), pushButtonSkip, SLOT(click()));
        //We've hit the end, show the congrats pane
        stackedWidget->setCurrentIndex(2);
    }
}

void DlgControllerLearning::prev() {
    iCurrentControl--;
    if (iCurrentControl >= 0)
    {
        MixxxControl control = m_controlsToMap[iCurrentControl];
        emit(learn(control));
        QString count = QString(tr("(Control %1 of %2)")).arg(iCurrentControl+1)
                        .arg(m_controlsToMap.size()+1);
        labelMixxxControl->setText(control.description()+"\n"+count);
        pushButtonSkip->setText(tr("Skip"));
        labelMappedTo->setText("");

        //We've hit the start, don't let the user go back anymore.
        if (iCurrentControl == 0)
            pushButtonPrevious->setEnabled(false);
    }

}

void DlgControllerLearning::controlMapped(QString message) {
    labelMappedTo->setText(tr("Successfully mapped to: ") + message);

    //Set the label on the "Skip" button to "Next" because we're proceeding
    //instead of "skipping" now. Makes more sense, I think...
    pushButtonSkip->setText(tr("Next"));
    //next();
}
