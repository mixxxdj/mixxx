#include "ladspabackend.h"

LADSPABackend::LADSPABackend() {
	qDebug() << "FXUNITS: LADSPABackend: " << this;

	PluginIDSequence = 0;
	m_monoBufferSize = 0;

}

LADSPABackend::~LADSPABackend() {
	// TODO Auto-generated destructor stub
}

/* LADSPABackend::loadPlugins
 * For each plugin available on LADSPALoader (all LADSPA plugins from all LADSPA libraries).
 * We will create a EffectsUnitsPlugin with all its Ports initialized using the LADSPA descriptor.
 * This will also initialize a couple of internal objects for storing that data.
 */
void LADSPABackend::loadPlugins(){

	EffectsUnitsPlugin * plugin;
	LADSPAPlugin * ladspaplugin;
	EffectsUnitsPort * port;

	m_LADSPALoader = new LADSPALoader();

	int i = 0;
	ladspaplugin = m_LADSPALoader->getByIndex(i);

	while (ladspaplugin != NULL){

		if (ladspaplugin->isSupported()){

			plugin = new EffectsUnitsPlugin(this, new QString(ladspaplugin->getLabel()), PluginIDSequence++);

			int j = 0;
			int port_count = ladspaplugin->getDescriptor()->PortCount;
			for (j = 0; j < port_count; j++){
				port = new EffectsUnitsPort();

				if (LADSPA_IS_PORT_AUDIO(ladspaplugin->getDescriptor()->PortDescriptors[j])){
					port->isAudio = true;
				} else {
					port->isAudio = false;
					port->Max = ladspaplugin->getDescriptor()->PortRangeHints[j].UpperBound;
					port->Min = ladspaplugin->getDescriptor()->PortRangeHints[j].LowerBound;
					port->Def = port->Min;
					port->Name = new QString(ladspaplugin->getDescriptor()->PortNames[j]);
				}

				plugin->addPort(port);
			}

			m_BackendPlugins.push_back(plugin);
			m_LADSPAPlugin.push_back(ladspaplugin);
			m_LADSPAInstance.push_back(NULL);
			m_PluginLADSPAControl.push_back(NULL);

		}

		ladspaplugin = m_LADSPALoader->getByIndex(++i);

		}

}

/* LADSPABackend::connect
 * Given a PluginID, Port and Value.
 * We'll update this plugin's Port with Value.
 * Updates the LADSPAControl variables of the LADSPAInstance.
 */
void LADSPABackend::connect(int PortID, float Value, int PluginID){
	m_pControlbeingUpdated = m_PluginLADSPAControl.at(PluginID);
	m_pControlbeingUpdated->at(PortID)->setValue(Value);
}

/* LADSPABackend::process
 * Given a PluginID,
 * We'll update all the LADSPA port values, using the control objects from the plugin,
 * Then we will use all our LADSPAMagic to process the audio samples.
 */
void LADSPABackend::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, int PluginID, double WetDry){
	/* Invalid PluginID or plugin with asymmetrical ports (uninstantiable), NOP */
	if (PluginID >= PluginIDSequence || m_LADSPAInstance.at(PluginID) == NULL){ return; }
	//qDebug() << "FXUNITS: LADSPABackend: Processing: " << m_BackendPlugins.at(PluginID)->getName();


	/* Initial Set-up, creating workspace. */
	if (m_monoBufferSize != iBufferSize/2){
		m_monoBufferSize = iBufferSize/2;

		LADSPAControl::setBufferSize(m_monoBufferSize);

		m_pBufferLeft[0] = new CSAMPLE[m_monoBufferSize];
		m_pBufferLeft[1] = new CSAMPLE[m_monoBufferSize];
		m_pBufferRight[0] = new CSAMPLE[m_monoBufferSize];
		m_pBufferRight[1] = new CSAMPLE[m_monoBufferSize];
	}

    for (int i = 0; i < m_monoBufferSize; i++)
    {
        m_pBufferLeft[0][i] = pIn[2 * i];
        m_pBufferRight[0][i] = pIn[2 * i + 1];
    }

    /* Process Audio Signals: */
    m_pInstancebeingProcessed = m_LADSPAInstance.at(PluginID);
	if (m_pInstancebeingProcessed->isInplaceBroken() || WetDry < 1.0)
	{
		m_pInstancebeingProcessed->process(m_pBufferLeft[0], m_pBufferRight[0], m_pBufferLeft[1], m_pBufferRight[1], m_monoBufferSize);
		//qDebug() << "FXUNITS: LADSPABackend::process: INP: " << *m_pBufferLeft[0] << "OUT: " << *m_pBufferLeft[1] << "BUF IPB: " << iBufferSize;

		for (int i = 0; i < m_monoBufferSize; i++)
		{
			m_pBufferLeft [0][i] = m_pBufferLeft [0][i] * (1-WetDry) + m_pBufferLeft [1][i] * WetDry; // TODO - Make Dry/Wet
			m_pBufferRight[0][i] = m_pBufferRight[0][i] * (1-WetDry) + m_pBufferRight[1][i] * WetDry;
		}
	} else {
		//qDebug() << "FXUNITS: LADSPABackend::process: IN: " << *m_pBufferLeft[0];
		m_pInstancebeingProcessed->process(m_pBufferLeft[0], m_pBufferRight[0], m_pBufferLeft[0], m_pBufferRight[0], m_monoBufferSize);
		//qDebug() << "FXUNITS: LADSPABackend::process: OUT: " << *m_pBufferLeft[0];
	}


	/* Creating Final Output */
	CSAMPLE * pOutput = (CSAMPLE *)pOut;
	for (int j = 0; j < m_monoBufferSize; j++)
	{
		pOutput[2 * j]     = m_pBufferLeft [0][j];
		pOutput[2 * j + 1] = m_pBufferRight[0][j];
	}
}

/* LADSPABackend::activatePlugin
 * Given a valid PluginID of an unactivated plugin, this is what we're going to do:
 * Turn LADSPAPlugin into LADSPAInstance (which has process())
 * Connect the ports of the instance to LADSPAControls, so we can tweak values
 */
void LADSPABackend::activatePlugin(int PluginID){
	qDebug() << "FXUNITS: PLUGINID:" << PluginID;
	if (m_LADSPAInstance.at(PluginID) == NULL && PluginID < PluginIDSequence){

		/* Instantiates the plugin, so we can process it */
		EffectsUnitsPlugin * fxplugin = m_BackendPlugins.at(PluginID);
		LADSPAInstance * instance = m_LADSPAPlugin.at(PluginID)->instantiate(0);

		/* Plugin with asymmetrical in/out ports, cant process() */
		if (instance == NULL) { qDebug() << "FXUNITS: LADSPABackend: Assymetrical ports!!! ABORT!"; return; }

		m_LADSPAInstance.replace(PluginID, instance);
		qDebug() << "FXUNITS: INSTANCE:" << instance;

		/* Handle plugins ports */
		QList<EffectsUnitsPort *> * ports = fxplugin->getPorts();
		QList<LADSPAControl *> * controls = new QList<LADSPAControl *>();
		LADSPAControl * current;

		int size = ports->size();
		for (int i = 0; i < size; i++){
			/* If its an audio port, it doesnt need a control */
			if (ports->at(i)->isAudio){
				controls->push_back(NULL);

			/* Creates a new LADSPAControl, connects its buffer to the plugin, assign default value */
			} else {
				current = new LADSPAControl();
				current->setValue(ports->at(i)->Def);
				instance->connect(i, current->getBuffer());
				controls->push_back(current);
			}
		}
		/* Adds the list of controls to be processed */
		m_PluginLADSPAControl.replace(PluginID, controls);

		qDebug() << "FXUNITS: LADSPABackend: Activating: " << fxplugin->getName();

	}
}

void LADSPABackend::deactivatePlugin(int PluginID){
	//TODO - deactivePlugin, hard to determine wether is it safe or not.

}

