#!/usr/bin/python3


# README
#
# 1. Change the following string to the absolute path of the directory
#    containing the *kbd.cfg files:
KBD_CFG_FILES_DIRECTORY = "/home/tomasito/Development/Mixxx/mixxx/res/keyboard"
#
#    Note: Only files with a valid language language code as filename will be
#          parsed. For example: en_US.kbd.cfg, es_ES.kbd.cfg or de_CH.kbd.cfg
#
# 2. Change the following string to the absolute path of the directory to
#    you want to save the generated keyboard preset:
TARGET_DIRECTORY = "/home/tomasito/Desktop/test"
#
# 3. Change the following string to the preset name. Name this whatever you
#    like:
PRESET_NAME = "Default-mapping"
#
#    Note: The preset name will be the name you will see when the preset is
#          listed in Mixxx under preferences -> controllers -> keyboard ->
#          presets.
#
#          The preset name will be also reflected in the filename. If your
#          preset name is "foo", the generated file will be:
#          {TARGET_DIRECTORY}/"foo.kbd.xml"
#
# 4. Run this file. (with PYTHON3 or higher!!)


import sys
import os
import re
import collections
import configparser
import json
from xml.etree import ElementTree as ET
from xml.dom import minidom

class KeyboardParser:
    def __init__(self, legacy_folder, target_folder, name,
                 multilang = True, legacy_file = None,
                 mixxx_version = "2.1.0+", author = "kbdcfg_to_kbdxml.py"):

        self.legacy_folder = legacy_folder
        self.target_folder = target_folder
        self.name = name
        self.multilang = multilang
        self.legacy_file = legacy_file
        self.legacy_extension = ".kbd.cfg"
        self.xml_extension = ".kbd.xml"
        self.mixxx_version = mixxx_version
        self.author = author
        self.description = "This preset was generated using kbdcfg_to_kbdxml.py"

        if not multilang and legacy_file is None:
            raise ValueError("Multilang is false, but legacy_file path is not given")

        if multilang:
            if not os.path.exists(legacy_folder):
                raise ValueError("Given legacy folder does not exist: \'" + legacy_folder + "\'")
            print("Reading legacy files from: " + legacy_folder)

            if not os.path.exists(target_folder):
                parent_folder = os.path.dirname(target_folder)
                if os.path.exists(parent_folder):
                    print("Warning: Given target folder does not exist: \'" + target_folder + "\'. Creating new folder...")
                    os.makedirs(target_folder)
                    print("Successfully created new folder: \'" + target_folder + "\'")
                else:
                    raise FileNotFoundError("Given target folder does not exist: \'" + target_folder + "\'" +
                                            "Neither does the parent. Not creating the folder.");
            print("Preset will be saved to: " + target_folder)


            legacy_mappings = self.parse_multi_lang_config()
            self.create_multi_lang_xml(legacy_mappings)

        else:
            # non multi-language parse not supported yet
            pass

    def parse_multi_lang_config(self):
        legacy_folder = self.legacy_folder
        legacy_extension = self.legacy_extension

        # Following the ISO 639-1 language code standard
        language_code_pattern = re.compile("^[a-z]{2}_[A-Z]{2}$")

        # Keys will hold the language code, values will hold config parsers
        # for the config file corresponding to that language code
        multi_lang_mappings = {}

        # Iterate over all files in legacy_folder
        for file_name in sorted(os.listdir(legacy_folder)):
            if file_name.endswith(legacy_extension):
                full_path = legacy_folder + '/' + file_name
                base_name = file_name[:-len(legacy_extension)]

                # Parse file and store it in multi_lang_mappings if filename
                # ends with kbd.cfg and starts with a valid language code
                if language_code_pattern.match(base_name):
                    print("Parsing " + file_name + "...")
                    parser = configparser.ConfigParser(allow_no_value=True)
                    parser.optionxform = str
                    parser.read(full_path)
                    multi_lang_mappings[base_name] = parser
                else:
                    print("Skipping " + file_name +
                          ". The filename doesn't match the ISO "
                          "639 1 language code standard")

        return multi_lang_mappings

    def create_multi_lang_xml(self, mappings):
        if not type(mappings) is dict:
            raise TypeError("Expected dictionary, but got " + type(mappings).__name__)

        # Create XML root element
        root = ET.Element('MixxxKeyboardPreset')
        root.set('schemaVersion', '1')
        root.set('mixxxVersion', self.mixxx_version)

        # Create info block
        info = ET.SubElement(root, 'info')
        ET.SubElement(info, 'name').text = self.name
        ET.SubElement(info, 'author').text = self.author
        ET.SubElement(info, 'description').text = self.description

        # Create controller block
        controller = ET.SubElement(root, 'controller')

        # Create keyboard layouts block
        keyboard_layouts = ET.SubElement(controller, 'keyboard-layouts')
        for layout in sorted(mappings.items()):
            ET.SubElement(keyboard_layouts, 'lang').text = layout[0]

        # Retrieve which groups and actions are defined and store them in
        # actions dictionary, where:
        #   key   = group name
        #   value = set, containing every action corresponding to the key group
        actions = {}

        # Iterate over all keyboard layouts and their mappings
        print("Reading data...")
        for mapping in mappings.items():
            parser = mapping[1]
            groups_in_this_mapping = parser.sections()

            for group in groups_in_this_mapping:
                if group not in actions: actions[group] = set()

                # Retrieve all actions in this group
                actions_in_this_group = set()
                for control in parser.items(group):
                    stripped_action = control[0].split()[0]
                    actions_in_this_group.add(stripped_action)

                # Add all actions to the actions set, if not already there
                actions[group].update(actions_in_this_group)

        # Add group blocks
        print("Adding groups...")
        for group in sorted(actions):
            group_element = ET.SubElement(controller, 'group')
            group_element.set('name', '[' + group + ']')
            print("\t" + '[' + group + ']')

            # Iterate over actions in this group
            for action in sorted(actions[group]):
                # Add control element node to current group node
                control_element = ET.SubElement(group_element, 'control')
                control_element.set('action', action)

                # Iterate over all keyboard layouts and find if
                # this action is defined
                for mapping in mappings.items():
                    parser = mapping[1]

                    # TODO(Tomasito) Don't build logic on exception catching!
                    try: parser.items(group)
                    except configparser.NoSectionError: continue

                    # Iterate through all actions in the group and see
                    # if the current action is defined in the current
                    # parser. (hence, the current keyboard layout)
                    for action_in_this_group in parser.items(group):
                        stripped_action_in_this_group = action_in_this_group[0].split()
                        this_action = stripped_action_in_this_group[0]
                        this_keyseq = None

                        try: this_keyseq = stripped_action_in_this_group[1]
                        except: this_keyseq = ""

                        if action == action_in_this_group[0].split()[0]:
                            keyseq_element = ET.SubElement(control_element, 'keyseq')
                            keyseq_element.set('lang', mapping[0])
                            keyseq_element.text = this_keyseq

        self.write_out(root)


    def write_out(self, root):
        full_path = self.target_folder + "/" + self.name + self.xml_extension
        if os.path.isfile(full_path):
            print("Warning: '" + full_path + "' already exists.\nOverriding that file assuming that it's ok :)\n")
        print("Writing to file: " + full_path + "\n...")
        xmlstr = minidom.parseString(ET.tostring(root)).toprettyxml()
        with open(full_path, "w") as f:
            f.write(xmlstr)

#-----------------------------------------------
# Change the strings bellow and run this script
#-----------------------------------------------
KeyboardParser(
    KBD_CFG_FILES_DIRECTORY, # <--- Path to legacy files like en_US.kbd.cfg
    TARGET_DIRECTORY,        # <--- Target path. The kbd.xml file will be stored here
    PRESET_NAME              # <--- Preset name
)
