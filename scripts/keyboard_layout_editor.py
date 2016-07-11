import os.path
import ntpath
import codecs
from tkinter import *
from xml.dom import minidom
from tkinter import filedialog
from sys import platform as _platform
from xml.etree import ElementTree
from xml.etree.ElementTree import Element, SubElement, tostring


def main():
    app = KeyboardLayoutEditor()
    app.mainloop()


class KeyboardLayoutEditor(Tk):
    TITLE = "Keyboard layouts editor"

    SHORTCUTS = {
        "new": {
            "event_name": "<Control-n>",
            "accelerator": "Ctrl+N"},
        "open": {
            "event_name": "<Control-o>",
            "accelerator": "Ctrl+O"},
        "save": {
            "event_name": "<Control-s>",
            "accelerator": "Ctrl+S"},
        "quit": {
            "event_name": "<Control-q>",
            "accelerator": "Ctrl+Q"}
    }

    @staticmethod
    def center_widget_to_screen(widget, width, height):
        ws = widget.winfo_screenwidth()
        hs = widget.winfo_screenheight()
        widget.geometry('%dx%d+%d+%d' % (width, height,
                                         (ws / 2) - (width / 2),
                                         (hs / 2) - (height / 2)))

    def __init__(self, *args, **kwargs):
        Tk.__init__(self, *args, **kwargs)
        self.wm_title(KeyboardLayoutEditor.TITLE)

        self.mainframe = MainFrame(self, app=self)
        self.sidebarframe = SideBarFrame(self.mainframe, app=self)
        self.workspaceframe = WorkspaceFrame(self.mainframe)
        self.dlgkeyboard = DlgKeyboard(self.workspaceframe, app=self)

        self.pack()
        self.update_idletasks()

        KeyboardLayoutEditor.center_widget_to_screen(
            self,
            width=self.winfo_width(),
            height=self.winfo_height()
        )

        # Full path of current XML file
        self.file_path = None

        # Name of current XML file
        self.file_name = None

        # Element tree of current XML file
        self.tree = None

        # Root element
        self.root = None

        # List of instances of KeyboardLayout
        self.layouts = None

        # The layout that is currently selected and displayed
        self.selected_layout = None

        # Load new file and init all member variables
        self.new_file()

        # Set shortcuts
        self.bind_all(KeyboardLayoutEditor.SHORTCUTS['new']['event_name'], lambda e: self.new_file())
        self.bind_all(KeyboardLayoutEditor.SHORTCUTS['save']['event_name'], lambda e: self.save_file())
        self.bind_all(KeyboardLayoutEditor.SHORTCUTS['open']['event_name'], lambda e: self.open_file())
        self.bind_all(KeyboardLayoutEditor.SHORTCUTS['quit']['event_name'], lambda e: self.quit())

        if _platform == "linux" or _platform == "linux2":
            os.system('xset r off')

        self.protocol("WM_DELETE_WINDOW", self.quit)

    def quit(self):
        print("Quitting...")
        self.destroy()

        if _platform == "linux" or _platform == "linux2":
            os.system('xset r on')

        sys.exit(0)

    def pack(self):
        self.sidebarframe.pack(side=LEFT, pady=0, fill=Y, expand=1)
        self.workspaceframe.pack(side=RIGHT)
        self.dlgkeyboard.pack(padx=18, pady=18)
        self.mainframe.pack(pady=0, padx=0)

    def new_file(self):
        self.file_path = None
        self.file_name = "Untitled.xml"
        self.tree = None
        self.root = Element('KeyboardLayoutTranslations')
        self.layouts = []
        self.selected_layout = None
        self.wm_title(KeyboardLayoutEditor.TITLE + " - " + self.file_name)
        self.dlgkeyboard.clear_all()
        self.sidebarframe.update_layout_list(self.layouts)

    def open_file(self):
        self.file_path = filedialog.askopenfilename(
            filetypes=(("XML Files", ".xml"), ("All Files", "*")),
            title="Choose a keyboard layout XML file"
        )

        if self.file_path == ():
            return

        if not os.path.isfile(self.file_path):
            print("File doesn't exist: " + self.file_path)
            self.file_path = None
            return

        print("Opening file: " + self.file_path)
        with codecs.open(self.file_path, 'r', "utf-8") as f:
            self.tree = ElementTree.parse(f)

        # Get KeyboardLayoutTranslations element, which holds all
        # information and is basically the root element
        root = None
        for element in self.tree.getroot().iter():
            if element.tag == 'KeyboardLayoutTranslations':
                root = element
                break
        self.root = root

        # Retrieve layouts element
        layouts_element = root.find('layouts')

        if not layouts_element:
            print("Couldn't retrieve layout list, no <layouts> element "
                  "in loaded XML. Pleas load in an other XML file.")
            return

        file_name = self.file_path
        self.wm_title(KeyboardLayoutEditor.TITLE + ' - ' + ntpath.basename(file_name))

        # Reset previous layouts
        self.layouts = []

        for layout in layouts_element.iter('lang'):
            name = layout.text
            if not KeyboardLayout.validate_layout_name(name):
                print("Layout name: " + name + " is not a valid language code name. Not loading this layout.")
                continue
            self.layouts.append(
                KeyboardLayout(name=name, root=root)
            )

        # Update layout list in side bar
        self.sidebarframe.update_layout_list(self.layouts)

    @staticmethod
    def write_out(root, full_path):
        if os.path.isfile(full_path):
            print("Warning: '" + full_path + "' already exists.\nOverriding that file assuming that it's ok :)\n")
        print("Writing to file: " + full_path + "\n...")
        xmlstr = minidom.parseString(tostring(root)).toprettyxml(indent="  ")
        with codecs.open(full_path, "w", "utf-8") as f:
            f.write(xmlstr)
        print("Saved file to: " + full_path)

    def save_file(self):
        path = self.file_path
        xml = self.create_xml()
        if not path:
            try:
                path = filedialog.asksaveasfile(mode='w', defaultextension=".xml").name
            except AttributeError:
                return
            if not path:
                return
            print(path)
        self.write_out(xml, path)

        self.file_path = path
        self.wm_title(KeyboardLayoutEditor.TITLE + ' - ' + ntpath.basename(self.file_path))

    def create_xml(self):
        # Create XML root element
        root = Element('KeyboardLayoutTranslations')

        # Create layouts block
        layouts = SubElement(root, 'layouts')
        for layout in self.layouts:
            SubElement(layouts, 'lang').text = layout.name

        # Retrieve all key scan codes, used by all layouts
        key_ids = set()
        for layout in self.layouts:
            key_ids.update(
                layout.get_key_ids()
            )

        for key_id in key_ids:
            key_element = SubElement(root, 'key')
            key_element.set('key_id', str(key_id))

            for layout in self.layouts:
                layout_name = layout.name
                keyboard_key = layout.find(key_id)

                key_char = keyboard_key.get_char(modifier=KeyboardKey.MODIFIERS.NONE)
                if key_char:
                    char = key_char.char
                    char_element = SubElement(key_element, 'char')
                    char_element.text = char
                    char_element.set('lang', layout_name)
                    char_element.set('modifier', 'NONE')

                key_char_shift = keyboard_key.get_char(modifier=KeyboardKey.MODIFIERS.SHIFT)
                if key_char_shift:
                    char = key_char_shift.char
                    key_char_shift = SubElement(key_element, 'char')
                    key_char_shift.text = char
                    key_char_shift.set('lang', layout_name)
                    key_char_shift.set('modifier', 'SHIFT')

        return root

    def add_layout(self, name):
        if not KeyboardLayout.validate_layout_name(name):
            print("Layout name: " + name + " is not a valid language code name. Not loading this layout.")
            return
        self.layouts.append(
            KeyboardLayout(name=name, root=self.root)
        )
        self.sidebarframe.update_layout_list(self.layouts)
        self.dlgkeyboard.update_keys()

    def get_layout(self, layout_name):
        for layout in self.layouts:
            if layout.name == layout_name:
                return layout
        print("No such layout: " + layout_name)
        return None

    def remove_layout(self, layout):
        self.layouts.remove(layout)
        self.sidebarframe.update_layout_list(self.layouts)

        if not self.layouts:
            self.selected_layout = None

        self.dlgkeyboard.update_keys()

    def select_layout(self, layout_name):
        layout = self.get_layout(layout_name)
        if not bool(layout):
            return

        self.selected_layout = layout
        self.dlgkeyboard.keyboard_layout = layout
        self.dlgkeyboard.update_keys()
        print("Selected layout: " + layout_name)


class MainFrame(Frame):
    def __init__(self, *args, app=None, **kwargs):
        Frame.__init__(self, *args, **kwargs)
        self.app = app

        # Setup main menu bar
        self.menubar = Menu(self)
        menu = Menu(self.menubar, tearoff=0)
        self.menubar.add_cascade(label="File", menu=menu)

        menu.add_command(
            label="New", command=lambda: app.new_file(),
            accelerator=KeyboardLayoutEditor.SHORTCUTS['new']['accelerator']
        )
        menu.add_command(
            label="Open", command=lambda: app.open_file(),
            accelerator=KeyboardLayoutEditor.SHORTCUTS['open']['accelerator']
        )
        menu.add_command(
            label="Save", command=lambda: app.save_file(),
            accelerator=KeyboardLayoutEditor.SHORTCUTS['save']['accelerator']
        )
        menu.add_command(
            label="Quit", command=lambda: app.quit(),
            accelerator=KeyboardLayoutEditor.SHORTCUTS['quit']['accelerator']
        )

        self.master.config(menu=self.menubar)


class SideBarFrame(Frame):
    def __init__(self, *args, app=None, **kwargs):
        Frame.__init__(self, *args, **kwargs)
        self.app = app

        buttons = Frame(self)
        Button(buttons, command=self.on_add_button_clicked, text="Add").pack(side=LEFT, fill=X, expand=1)
        Button(buttons, command=self.on_remove_button_clicked, text="Remove").pack(side=LEFT, fill=X, expand=1)
        buttons.pack(fill=BOTH)

        self.listbox = Listbox(self)
        self.listbox.pack(fill=BOTH, expand=1)

        self.listbox.bind('<<ListboxSelect>>', self.on_listbox_item_selected)

    def on_add_button_clicked(self, e=None):
        d = LayoutNameDialog(self)
        self.wait_window(d.top)

    def on_remove_button_clicked(self, e=None):
        selected_layout = self.app.selected_layout
        self.app.remove_layout(selected_layout)

    def update_layout_list(self, layouts):
        self.listbox.delete(0, END)
        for layout in layouts:
            self.listbox.insert(END, layout.name)

        if self.listbox.size() > 0:
            self.listbox.select_set(0)
            self.listbox.event_generate("<<ListboxSelect>>")

    def on_listbox_item_selected(self, e):
        widget = e.widget
        try:
            index = int(widget.curselection()[0])
        except IndexError:
            return
        layout_name = widget.get(index)
        self.app.select_layout(layout_name)


class LayoutNameDialog:
    TITLE = "Add layout"

    def __init__(self, parent):
        self.app = parent.app
        top = self.top = Toplevel(parent)
        top.wm_title(LayoutNameDialog.TITLE)

        Label(top, text="Layout name:").pack(fill=BOTH)
        self.entry = Entry(top)
        self.entry.pack(padx=75)

        self.entry.bind('<Return>', self.ok)
        Button(top, text="OK", command=self.ok).pack(pady=3)

        top.update_idletasks()
        KeyboardLayoutEditor.center_widget_to_screen(
            top,
            width=top.winfo_width(),
            height=top.winfo_height()
        )

        self.entry.focus_set()

    def ok(self, e=None):
        name = self.entry.get()
        if not KeyboardLayout.validate_layout_name(name):
            print("The entered name: '" + name + "' is not valid. Please enter a name language code starting with a "
                                                 "two letter ISO 639-1 language code, followed by an underscore, "
                                                 "ending with two or three letter uppercase ISO 3166 country code.\n"
                                                 "Example: 'en_US', or 'es_ES'")
            return
        self.app.add_layout(name)
        self.top.destroy()


class KeyboardKey:
    """ Class representing one physical character key as seen here:
    https://en.wikipedia.org/wiki/Keyboard_layout#/media/File:ISO_keyboard_(105)_QWERTY_UK.svg

    This class contains information about which character is bound
    to this keyboard key, also for different modifiers"""

    class MODIFIERS:
        """ Modifier enum, following the Qt::KeyboardModifier standard, found here:
        http://doc.qt.io/qt-5/qt.html#KeyboardModifier-enum """
        NONE, SHIFT, CTRL, ALT, META, KEYPAD = range(1, 7)

        @staticmethod
        def is_valid_modifier(modifier):
            return modifier in range(1, 3)

    class KeyChar:
        def __init__(self, modifier, char, dead_key=False):
            self.modifier = modifier
            self.char = char
            self.dead_key = dead_key

    def __init__(self, key_id):
        self.key_id = key_id

        # A list containing KeyChar objects. For example, for the a key, it will contain:
        #   - One KeyChar with modifier == NONE and char = 'a'
        #   - One KeyChar with modifier == SHIFT and char = 'A'
        self._key_chars = []

    def get_char(self, modifier, verbose=True):
        if not KeyboardKey.MODIFIERS.is_valid_modifier(modifier):
            if verbose:
                print("Given modifier: " + modifier + " is not valid.")
            return

        for key_char in self._key_chars:
            if key_char.modifier == modifier:
                return key_char

        if verbose:
            print("Keyboard key with key_id '" + str(self.key_id) +
                  "' has not a key char set with modifier: " + str(modifier))
        return None

    def set_key_char(self, modifier, char, dead_key=False):
        if not KeyboardKey.MODIFIERS.is_valid_modifier(modifier):
            print("Given modifier: " + modifier + " is not valid.")
            return

        # Check if there is already a KeyChar with the given modifier
        key_char = self.get_char(modifier=modifier, verbose=False)

        if key_char:
            key_char.char = char
            key_char.dead_key = dead_key
        else:
            self._key_chars.append(KeyboardKey.KeyChar(
                modifier=modifier,
                char=char,
                dead_key=dead_key
            ))


class KeyboardLayout:
    NAME_VALIDATION_PATTERN = re.compile("^[a-z]{2}_[A-Z]{2,3}$")

    @staticmethod
    # Check if name has the correct format, described here: http://doc.qt.io/qt-4.8/qlocale.html#QLocale-2
    # Name has to start with a two letter ISO 639-1 language code ...
    # ... followed by an underscore ...
    # ... followed by a three letter uppercase ISO 3166 country code
    #
    # Note: This validation is not full-prove: qw_ZXC will pass.
    def validate_layout_name(name):
        return KeyboardLayout.NAME_VALIDATION_PATTERN.match(name)

    def __init__(self, name="", root=None):
        # Keyboard layout name
        self.name = name

        # Mapping data for this keyboard layout, contains instances of KeyboardKey.
        self._data = []

        # Parse XML and store it into self._data
        self.parse_xml(root)

    def parse_xml(self, root):
        for key in root.iter('key'):
            key_id = key.get('key_id')
            if not key_id:
                print(
                    "Skipping key, no scan code defined. Make sure that all key "
                    "elements have a 'key_id' attribute telling the key_id as "
                    "described here: http://www.barcodeman.com/altek/mule/kb102.gif")
                break

            for char in key.iter('char'):
                lang = char.get('lang')
                if lang == self.name:
                    modifier_attr = char.get('modifier')
                    if modifier_attr == "NONE":
                        modifier = KeyboardKey.MODIFIERS.NONE
                    elif modifier_attr == "SHIFT":
                        modifier = KeyboardKey.MODIFIERS.SHIFT
                    else:
                        modifier = -1

                    self.update_key(
                        key_id=int(key_id),
                        modifier=int(modifier),
                        char=char.text
                    )

    def find(self, key_id):
        for key in self._data:
            if key.key_id == key_id:
                return key
        return None

    def update_key(self, key_id, modifier, char, dead_key=False):
        key = self.find(key_id)
        if not key:
            print("Can't update key with key_id '" + str(key_id) +
                  "'. key_id doesnt't exist. Creating new one...")
            key = KeyboardKey(key_id)
            self._data.append(key)
        key.set_key_char(modifier=modifier, char=char, dead_key=dead_key)

    def get_key_ids(self):
        key_ids = set()
        for key in self._data:
            key_ids.add(key.key_id)
        return key_ids

    def get_key_id(self, keyseq, modifier):
        # Retrieve key_id
        split_keyseq = keyseq.split('+')
        char = split_keyseq[-1] if split_keyseq else ""

        # Make sure that the character is a lower-case character if shift is not pressed and
        # is an upper-case character when shift is pressed so that it can be found in _data
        char = char.upper() if modifier == KeyboardKey.MODIFIERS.SHIFT else char.lower()

        key_ids = []
        for key in self._data:
            if key.get_char(modifier).char == char:
                key_ids.append(key.key_id)
        key_id = key_ids[0] if len(key_ids) == 1 \
            else "TODO: Set key_id for " + char + " (please choose between one of these: " + str(key_ids) + ")"

        # Check if this key is universal (for example: F-keys or Space)
        if KeyboardLayout.is_universal_key(char):
            key_id = "universal_key"

        return key_id

    @staticmethod
    def is_universal_key(key):
        universal_keys = [
            "LEFT", "UP", "RIGHT", "DOWN",
            "SPACE", "RETURN", "F1", "F2",
            "F3", "F4", "F5", "F6", "F7",
            "F8", "F9", "F10", "F11", "F12"]

        if key.upper() in universal_keys:
            return True
        else:
            return False


class WorkspaceFrame(Frame):
    def __init__(self, *args, **kwargs):
        Frame.__init__(self, *args, **kwargs)

        self.configure(background='#16a085')


class DlgKeyboard(Frame):
    BG_COLOR = '#1abc9c'

    def __init__(self, *args, app=None, **kwargs):
        Frame.__init__(self, *args, **kwargs)
        self.keys = []
        self.app = app

        self.configure(
            background=DlgKeyboard.BG_COLOR,
            highlightbackground=DlgKeyboard.BG_COLOR,
            borderwidth=3
        )

        # Setup context menu
        self.context_menu = Menu(self, tearoff=0)
        self.context_menu.add_command(label="Dead key", command=lambda: self.active_key.set_dead_key())
        self.context_menu.add_command(label="Reset", command=lambda: self.active_key.set_char(""))

        # Current active key (key that has focus)
        self.active_key = None

        # F-keys
        f_keys_row = Frame(self)
        f_keys_row.configure(background=DlgKeyboard.BG_COLOR)
        DlgKeyboardKey(f_keys_row, key_id=110, char="ESC", state=DISABLED)
        for i in range(112, 124):
            char = "F" + str(i - 111)
            DlgKeyboardKey(f_keys_row, key_id=i, width=4, char=char, state=DISABLED)
        f_keys_row.grid(row=1, column=1, sticky='we')

        # Numeric keys
        numeric_keys_row = Frame(self)
        numeric_keys_row.configure(background=DlgKeyboard.BG_COLOR)
        for i in range(1, 14):
            key = DlgKeyboardKey(numeric_keys_row, key_id=i, dlg_keyboard=self, state=DISABLED)
            self.keys.append(key)
        self.keys.append(DlgKeyboardKey(numeric_keys_row, key_id=42, dlg_keyboard=self, state=DISABLED))
        DlgKeyboardKey(numeric_keys_row, key_id=15, width=5, char="Backspace", state=DISABLED)
        numeric_keys_row.grid(row=2, column=1, sticky='we')

        # Character keys (qwertyuiop[] on en_US)
        letters_keys_row_1 = Frame(self)
        letters_keys_row_1.configure(background=DlgKeyboard.BG_COLOR)
        DlgKeyboardKey(letters_keys_row_1, key_id=16, width=5, char="Tab", state=DISABLED)
        for i in range(17, 29):
            key = DlgKeyboardKey(letters_keys_row_1, key_id=i, dlg_keyboard=self, state=DISABLED)
            self.keys.append(key)
        self.keys.append(DlgKeyboardKey(letters_keys_row_1, key_id=42, width=7, dlg_keyboard=self, state=DISABLED))
        letters_keys_row_1.grid(row=3, column=1, sticky='we')

        # Character keys (asdfghjkl;'\ on en_US)
        letters_keys_row_2 = Frame(self)
        letters_keys_row_2.configure(background=DlgKeyboard.BG_COLOR)
        DlgKeyboardKey(letters_keys_row_2, key_id=30, width=7, char="Caps Lock", state=DISABLED)
        for i in range(31, 43):
            key = DlgKeyboardKey(letters_keys_row_2, key_id=i, dlg_keyboard=self, state=DISABLED)
            self.keys.append(key)
        DlgKeyboardKey(letters_keys_row_2, key_id=43, width=5, char="Enter", state=DISABLED)
        letters_keys_row_2.grid(row=4, column=1, sticky='we')

        # Character keys (\zxcvbnm,./ on en_US)
        letters_keys_row_3 = Frame(self)
        letters_keys_row_3.configure(background=DlgKeyboard.BG_COLOR)
        self.shift_l = DlgKeyboardKey(letters_keys_row_3, key_id=44, width=5, char="Shift", state=DISABLED)
        for i in range(45, 56):
            key = DlgKeyboardKey(letters_keys_row_3, key_id=i, dlg_keyboard=self, state=DISABLED)
            self.keys.append(key)
        self.shift_r = DlgKeyboardKey(letters_keys_row_3, key_id=57, width=11, char="Shift", state=DISABLED)
        letters_keys_row_3.grid(row=5, column=1, sticky='we')

        self.shift_pressed = False

    def update_keys_state(self, state):
        for key in self.keys:
            key.config(state=state)
            char = key.cget('text')
            key.set_char(char)

    def reset_keys(self):
        for key in self.keys:
            key.set_char('')

    def update_keys(self):
        layout = self.app.selected_layout
        modifier = KeyboardKey.MODIFIERS.SHIFT if self.shift_pressed else KeyboardKey.MODIFIERS.NONE

        if layout:
            self.update_keys_state(NORMAL)
        else:
            self.update_keys_state(DISABLED)
            self.reset_keys()
            return

        relief = SUNKEN if modifier == KeyboardKey.MODIFIERS.SHIFT else RAISED
        self.shift_l.config(relief=relief)
        self.shift_r.config(relief=relief)

        for dlg_key in self.keys:
            key = layout.find(dlg_key.key_id)
            if key:
                key_char = key.get_char(modifier)
                if key_char:
                    char = key_char.char
                    dead_key = key_char.dead_key
                else:
                    char = ""
                    dead_key = False
            else:
                char = ""
                dead_key=False
            dlg_key.set_char(char, dead_key=dead_key)

    def clear_all(self):
        for key in self.keys:
            key.set_char("")

    def set_keys_with_same_key_id_as(self, p_key):
        char = p_key.cget('text')
        for i_key in self.keys:
            if i_key.key_id == p_key.key_id and i_key != p_key:
                i_key.set_char(char)

    def show_context_menu(self, x, y):
        self.context_menu.post(x, y)


class DlgKeyboardKey(Button):
    SIZE = {
        "width": 3,
        "height": 2
    }

    COLORS = {
        "background_color": "#434A55",
        "active_background_color": "#656D79",
        "background_color_key_set": "#8FC238",
        "active_background_color_key_set": "#A3D553",
        "disabled_color": "#CCD1DA",
        "dead_key_background_color": "#f1c40f",
        "dead_key_active_background_color": "#f39c12"
    }

    def __init__(self, *args, key_id=None, width=SIZE['width'], char=None, dlg_keyboard=None, **kwargs):
        Button.__init__(self, *args, width=width, height=DlgKeyboardKey.SIZE['height'], command=self.set_focus, **kwargs)
        self.set_char(char)
        self.pack_propagate(0)
        self.pack(side=LEFT, padx=1, pady=1, expand=1)
        if not key_id:
            print("Warning: no key_id given for this DlgKeyboardKey")
        self.key_id = key_id
        self.dlg_keyboard = dlg_keyboard
        self.key_id_label = Label(self, text=key_id, font=("Helvetica", 6))

        # Shift pressed / released check
        self.bind("<KeyPress-Shift_R>", lambda e: self.update_shift_state(True))
        self.bind("<KeyPress-Shift_L>", lambda e: self.update_shift_state(True))
        self.bind("<KeyRelease-Shift_R>", lambda e: self.update_shift_state(False))
        self.bind("<KeyRelease-Shift_L>", lambda e: self.update_shift_state(False))

        # Bind all other key-press events
        self.bind("<KeyPress>", self.on_key_press)

        # Show or hide key_id when mouse hover
        self.bind("<Enter>", lambda e: self.show_key_id())
        self.bind("<Leave>", lambda e: self.hide_key_id())

        # Bind button to show context menu
        self.bind("<Button-3>", self._popup_context_menu_command)

        self.dead_key = False

    def show_key_id(self):
        self.key_id_label.place(x=0, y=0)

    def hide_key_id(self):
        self.key_id_label.place_forget()

    def update_shift_state(self, pressed):
        if self.dlg_keyboard.shift_pressed == pressed:
            return
        self.dlg_keyboard.shift_pressed = pressed
        self.dlg_keyboard.update_keys()

    def set_char(self, char, dead_key=False):
        if not char:
            char = ""
        self.config(text=char, font=("Helvetica", 12))
        self.update_button_colors(bool(char), dead_key)

    def update_button_colors(self, key_set, dead_key):
        if self['state'] == DISABLED:
            self.configure(bg="#CCD1DA")
            return

        if dead_key:
            self.configure(
                fg="white",
                activeforeground="white",
                bg=DlgKeyboardKey.COLORS["dead_key_background_color"],
                activebackground=DlgKeyboardKey.COLORS["dead_key_active_background_color"]
            )
            return

        if key_set:
            self.configure(
                fg="black",
                activeforeground="black",
                bg=DlgKeyboardKey.COLORS["background_color_key_set"],
                activebackground=self.COLORS["active_background_color_key_set"]
            )
        else:
            self.configure(
                fg="white",
                activeforeground="white",
                bg=DlgKeyboardKey.COLORS["background_color"],
                activebackground=self.COLORS["active_background_color"]
            )

    def on_key_press(self, e):
        char = e.char

        # Check for DELETE key
        if e.keysym_num == 65535 or e.keysym_num == 65439:
            char = ""
        elif not char.strip():
            return

        # Check for CTRL key
        if e.keysym_num == 65507 or e.keysym_num == 65508:
            return

        self.set_char(char)

        key_id = self.key_id
        if not key_id:
            print("This key was not assigned a key_id and the key: " +
                  char + "can not be saved to the current keyboard layout")
            self.set_char("")
            return

        app = self.dlg_keyboard.app
        layout = app.selected_layout
        if not layout:
            print("Layout is None, not setting key")
            self.set_char("")
            return

        modifier = KeyboardKey.MODIFIERS.SHIFT if self.dlg_keyboard.shift_pressed else KeyboardKey.MODIFIERS.NONE
        layout.update_key(key_id=key_id, modifier=modifier, char=char)
        self.dlg_keyboard.set_keys_with_same_key_id_as(self)

        # Circular focus (when focus on last key, return focus to first key)
        if self != self.dlg_keyboard.keys[-1]:
            self.tk_focusNext().set_focus()
        else:
            self.dlg_keyboard.keys[0].set_focus()

        # If we don't invoke a shift down event on the next key, it won't
        # detect shift release event when we release the shift key
        if self.dlg_keyboard.shift_pressed:
            self.tk_focusNext().event_generate("<KeyPress-Shift_L>")
            self.tk_focusNext().event_generate("<KeyPress-Shift_R>")

    def set_focus(self):
        self.dlg_keyboard.active_key = self
        self.focus_set()

    def _popup_context_menu_command(self, event):
        state = self.cget('state')
        if state == DISABLED:
            return
        self.set_focus()

        self.dlg_keyboard.show_context_menu(event.x_root, event.y_root)

    def set_dead_key(self):
        key_id = self.key_id
        if not key_id:
            print("This key was not assigned a key_id and the key can not be marked as a dead key")
            return

        app = self.dlg_keyboard.app
        layout = app.selected_layout
        if not layout:
            print("Layout is None, not marking key as dead key")
            return

        modifier = KeyboardKey.MODIFIERS.SHIFT if self.dlg_keyboard.shift_pressed else KeyboardKey.MODIFIERS.NONE
        layout.update_key(key_id=key_id, modifier=modifier, char='', dead_key=True)
        self.dlg_keyboard.set_keys_with_same_key_id_as(self)

        self.set_char("", dead_key=True)


if __name__ == '__main__':
    main()
