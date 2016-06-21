#!/usr/bin/python3

"""
README

1. Open the terminal and cd to the directory containing the *kbd.cfg files
   that you want to translate.
   
2. Execute this file. The translated files will be stored per default in
   ./translated_legacy_to_xml_kbd. If you want to store the files in another
   directory, run the script, passing an absolute path as first parameter.

"""

import sys
import os
import configparser
from xml.etree import ElementTree as ET
from xml.dom import minidom

class KeyboardControllerTranslator:

    def __init__(self, kbdCfgPath, name):
        # Info variables, will be used when creating <info> block
        self.m_name = name
        self.m_author = 'Auto-generated'
        self.m_description = 'This keyboard controller preset was generated from ' + kbdCfgPath

        # Setup config parser
        self.m_kbdCfgPath = kbdCfgPath
        self.m_configParser = configparser.ConfigParser(allow_no_value=True)
        self.m_configParser.optionxform = str
        self.m_configParser.read(self.m_kbdCfgPath)
        
        self.initRootElement()
        self.addInfo()
        self.translate()

    def initRootElement(self, mixxxVersion = '2.1.0+'):
        self.m_rootElement = ET.Element('MixxxKeyboardPreset')
        self.m_rootElement.set('schemaVersion', '1')
        self.m_rootElement.set('mixxxVersion', mixxxVersion)

    def addInfo(self):
        # Creating <info> element
        infoElement = ET.SubElement(self.m_rootElement, 'info')
        name = ET.SubElement(infoElement, 'name')
        name.text = self.m_name

        # Adding author
        author = ET.SubElement(infoElement, 'author')
        author.text = self.m_author

        # Adding description
        description = ET.SubElement(infoElement, 'description')
        description.text = self.m_description

    def translate(self):
        controllerElement = ET.SubElement(self.m_rootElement, 'controller')
        
        for group in self.m_configParser.sections():
            
            # Creating <group> block
            groupElement = ET.SubElement(controllerElement, 'group')
            groupElement.set('name', '[' + group + ']')


            # Filling <group> block with <control> nodes
            for (info, x) in self.m_configParser.items(group):

                # Since kbd.cfg files only contain keys (no values),
                # all the info is stored in the value. Therefore we
                # have to split the value on a whitespace
                splittedInfo = info.split()
                action = splittedInfo[0]
                try:
                    keyseq = splittedInfo[1]
                except IndexError:
                    keyseq = ""

                # Create <control> node
                controlElement = ET.SubElement(groupElement, 'control')
                controlElement.set('action', action)
                controlElement.set('keyseq', keyseq)

    def writeOut(self, path):
        xmlstr = minidom.parseString(ET.tostring(self.m_rootElement)).toprettyxml()
        with open(path, "w") as f:
            f.write(xmlstr)



# ----------------
# |IMPLEMENTATION|
# ----------------

workingDirectory = os.getcwd()
legacyDirectory = workingDirectory;
xmlDirectory = ""

nArguments = len(sys.argv)

# If no arguments are given, set target directory to ./translated_legacy_to_xml_kbd
if (nArguments == 1):
    xmlDirectory = workingDirectory + "/translated_legacy_to_xml_kbd"

    if not os.path.exists(xmlDirectory):
        os.makedirs(xmlDirectory)

# If an argument is given, set target directory to that
elif (nArguments == 2):
    xmlDirectory = sys.argv[1]

    if not os.path.exists(xmlDirectory):
        print("The directory: '" + xmlDirectory + "' does not exist")
        exit()

for i in os.listdir(workingDirectory):
    if i.endswith(".kbd.cfg"):
        path = workingDirectory + '/' + i
        filename = os.path.splitext(i)[0] + '.xml'
        translator = KeyboardControllerTranslator(path, filename)
        translator.writeOut(xmlDirectory + '/' + filename)
