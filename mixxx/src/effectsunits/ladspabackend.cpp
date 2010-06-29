#include "ladspabackend.h"

LADSPABackend::LADSPABackend() {
	qDebug() << "FXUNITS: LADSPABackend: " << this;

	PluginIDSequence = 0;
	m_LADSPALoader = new LADSPALoader();

//	LADSPAPlugin * flanger = loader->getByLabel("Plate2x2");
//	LADSPAInstance * flangerInstance = flanger->instantiate(0);
//
//	m_test = flangerInstance;
//
//
//    zero = new LADSPAControl();
//    um = new LADSPAControl();
//    dois = new LADSPAControl();
//    tres = new LADSPAControl();
//
//    zero->setValue(0.799);
//    um->setValue(0.745);
//    dois->setValue(0.8);
//    tres->setValue(0.9);
//
//    m_test->connect(2, zero->getBuffer());
//    m_test->connect(3, um->getBuffer());
//    m_test->connect(4, dois->getBuffer());
//    m_test->connect(5, tres->getBuffer());

}

LADSPABackend::~LADSPABackend() {
	// TODO Auto-generated destructor stub
}

void LADSPABackend::loadPlugins(){

	EffectsUnitsPlugin * plugin;
	LADSPAPlugin * ladspaplugin;
	EffectsUnitsPort * port;

	int i = 0;
	ladspaplugin = m_LADSPALoader->getByIndex(i);

	while (ladspaplugin != NULL){

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
		m_LADPSAPlugin.push_back(ladspaplugin);
		m_LADSPAInstance.push_back(NULL);

		i++;

		ladspaplugin = m_LADSPALoader->getByIndex(i);
	}

}

void LADSPABackend::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize, int PluginID){
	//qDebug() << "FXUNITS: LADSPABackend: Processing: " << PluginID;

//	LADSPAControl::setBufferSize(256);

//	int m_monoBufferSize = iBufferSize/2;
//
//    m_pBufferLeft[0] = new CSAMPLE[m_monoBufferSize];
//    m_pBufferLeft[1] = new CSAMPLE[m_monoBufferSize];
//    m_pBufferRight[0] = new CSAMPLE[m_monoBufferSize];
//    m_pBufferRight[1] = new CSAMPLE[m_monoBufferSize];
//
//    for (int i = 0; i < m_monoBufferSize; i++)
//    {
//        m_pBufferLeft[0][i] = pIn[2 * i];
//        m_pBufferRight[0][i] = pIn[2 * i + 1];
//    }
//
//
//    zero->setValue(0.699);
//    um->setValue(0.5);
//    dois->setValue(0.7);
//    tres->setValue(0.7);
//
//    	if (m_test->isInplaceBroken())
//    	{
//    		m_test->process(m_pBufferLeft[0], m_pBufferRight[0], m_pBufferLeft[1], m_pBufferRight[1], m_monoBufferSize);
//    		qDebug() << "FXUNITS: LADSPABackend::process: INP: " << *m_pBufferLeft[0] << "OUT: " << *m_pBufferLeft[1] << "BUF IPB: " << iBufferSize;
//
//			for (int i = 0; i < m_monoBufferSize; i++)
//			{
//				m_pBufferLeft [0][i] = m_pBufferLeft [0][i] * 0.2 + m_pBufferLeft [1][i] * 0.8;
//				m_pBufferRight[0][i] = m_pBufferRight[0][i] * 0.2 + m_pBufferRight[1][i] * 0.8;
//			}
//    	} else {
//    		qDebug() << "FXUNITS: LADSPABackend::process: IN: " << *m_pBufferLeft[0];
//    	    m_test->process(m_pBufferLeft[0], m_pBufferRight[0], m_pBufferLeft[0], m_pBufferRight[0], m_monoBufferSize);
//    	    qDebug() << "FXUNITS: LADSPABackend::process: OUT: " << *m_pBufferLeft[0];
//    	}
//
//
//   CSAMPLE * pOutput = (CSAMPLE *)pOut;
//	for (int j = 0; j < m_monoBufferSize; j++)
//	{
//		pOutput[2 * j]     = m_pBufferLeft [0][j];
//		pOutput[2 * j + 1] = m_pBufferRight[0][j];
//	}
}

void LADSPABackend::activatePlugin(int PluginID){
	//TODO - Turn plugin into instance

}

void LADSPABackend::deactivatePlugin(int PluginID){
	//TODO - Turn plugin into instance

}

