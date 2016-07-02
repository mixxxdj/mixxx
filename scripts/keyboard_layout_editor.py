import os.path
import re
import ntpath
from tkinter import *
from xml.dom import minidom
from tkinter import filedialog
from xml.etree import ElementTree
from xml.etree.ElementTree import Element, SubElement, tostring


def main():
    app = KeyboardLayoutEditor()
    app.mainloop()


class KeyboardLayoutEditor(Tk):
    TITLE = "Keyboard layouts editor"

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
        with open(self.file_path, 'rt') as f:
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
        with open(full_path, "w") as f:
            f.write(xmlstr)
        print("Saved file to: " + full_path)

    def save_file(self):
        path = self.file_path
        xml = self.create_xml()
        if not path:
            path = filedialog.asksaveasfile(mode='w', defaultextension=".xml").name
            if not path:
                return
            print(path)
        self.write_out(xml, path)

    def create_xml(self):
        # Create XML root element
        root = Element('KeyboardLayoutTranslations')

        # Create layouts block
        layouts = SubElement(root, 'layouts')
        for layout in self.layouts:
            SubElement(layouts, 'lang').text = layout.name

        # Retrieve all key scan codes, used by all layouts
        scancodes = set()
        for layout in self.layouts:
            scancodes.update(
                layout.get_scancodes()
            )

        for scancode in scancodes:
            key_element = SubElement(root, 'key')
            key_element.set('scancode', str(scancode))

            for layout in self.layouts:
                layout_name = layout.name
                char = layout.find(scancode)

                char_element = SubElement(key_element, 'char')
                char_element.text = char
                char_element.set('lang', layout_name)

        return root

    def add_layout(self, name):
        if not KeyboardLayout.validate_layout_name(name):
            print("Layout name: " + name + " is not a valid language code name. Not loading this layout.")
            return
        self.layouts.append(
            KeyboardLayout(name=name, root=self.root)
        )
        self.sidebarframe.update_layout_list(self.layouts)

    def get_layout(self, layout_name):
        for layout in self.layouts:
            if layout.name == layout_name:
                return layout
        print("No such layout: " + layout_name)
        return None

    def remove_layout(self, layout):
        self.layouts.remove(layout)
        self.sidebarframe.update_layout_list(self.layouts)

    def select_layout(self, layout_name):
        layout = self.get_layout(layout_name)
        if not bool(layout):
            return

        self.dlgkeyboard.update_keys(layout)
        self.selected_layout = layout
        self.dlgkeyboard.keyboard_layout = layout
        print("Selected layout: " + layout_name)


class MainFrame(Frame):
    def __init__(self, *args, app=None, **kwargs):
        Frame.__init__(self, *args, **kwargs)
        self.app = app

        # Setup main menu bar
        self.menubar = Menu(self)
        menu = Menu(self.menubar, tearoff=0)
        self.menubar.add_cascade(label="File", menu=menu)
        menu.add_command(label="New", command=lambda: app.new_file())
        menu.add_command(label="Open", command=lambda: app.open_file())
        menu.add_command(label="Save", command=lambda: app.save_file())
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
        self.listbox.pack()

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
        index = 0
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

        # Dictionary containing layout data, where:
        #   key   = scan code (as described here http://www.barcodeman.com/altek/mule/kb102.gif)
        #   value = character bound to this key
        self._data = {}

        # Parse XML and store it into self.data
        self.parse_xml(root)

    def parse_xml(self, root):
        for key in root.iter('key'):
            scancode = key.get('scancode')
            if not scancode:
                print(
                    "Skipping key, no scan code defined. Make sure that all key "
                    "elements have a 'scancode' attribute telling the scancode as "
                    "described here: http://www.barcodeman.com/altek/mule/kb102.gif")
                break

            for char in key.iter('char'):
                lang = char.get('lang')
                if lang == self.name:
                    self._data[int(scancode)] = char.text

    def find(self, scancode):
        data = self._data
        if scancode in data:
            return self._data[scancode]
        else:
            return None

    def update_key(self, scancode, char):
        self._data[int(scancode)] = char

    def get_scancodes(self):
        scancodes = set()
        for scancode, char in self._data.items():
            scancodes.add(scancode)
        return scancodes


class WorkspaceFrame(Frame):
    def __init__(self, *args, **kwargs):
        Frame.__init__(self, *args, **kwargs)

        self.configure(background='#16a085')


class DlgKeyboard(Frame):
    def __init__(self, *args, app=None, **kwargs):
        Frame.__init__(self, *args, **kwargs)
        self.keys = []
        self.app = app

        # Numeric keys
        row_1 = Frame(self)
        for i in range(1, 14):
            key = DlgKeyboardKey(row_1, scancode=i, dlg_keyboard=self)
            self.keys.append(key)
        row_1.grid(row=1, column=1, sticky='we')

        # Character keys (qwertyuiop[] on en_US)
        row_2 = Frame(self)
        DlgKeyboardKey(row_2, scancode=16, width=4, char="Tab", state=DISABLED)
        for i in range(17, 29):
            key = DlgKeyboardKey(row_2, scancode=i, dlg_keyboard=self)
            self.keys.append(key)
        row_2.grid(row=2, column=1, sticky='we')

        # Character keys (asdfghjkl;'\ on en_US)
        row_3 = Frame(self)
        DlgKeyboardKey(row_3, scancode=16, width=6, char="Caps Lock", state=DISABLED)
        for i in range(31, 43):
            key = DlgKeyboardKey(row_3, scancode=i, dlg_keyboard=self)
            self.keys.append(key)
        row_3.grid(row=3, column=1, sticky='we')

        # Character keys (\zxcvbnm,./ on en_US)
        row_4 = Frame(self)
        DlgKeyboardKey(row_4, scancode=16, char="Shift", state=DISABLED)
        for i in range(45, 56):
            key = DlgKeyboardKey(row_4, scancode=i, dlg_keyboard=self)
            self.keys.append(key)
        row_4.grid(row=4, column=1, sticky='we')

    def update_keys(self, keyboardlayout):
        keys = self.keys
        for key in keys:
            char = keyboardlayout.find(key.scancode)
            key.set_char(char)

    def clear_all(self):
        for key in self.keys:
            key.set_char("")


class DlgKeyboardKey(Button):
    SIZE = {
        "width": 2,
        "height": 2
    }

    COLORS = {
        "background_color": "#434A55",
        "active_background_color": "#656D79",
        "background_color_key_set": "#8FC238",
        "active_background_color_key_set": "#A3D553",
        "disabled_color": "#CCD1DA"
    }

    def __init__(self, *args, scancode=None, width=2, char=None, dlg_keyboard=None, **kwargs):
        Button.__init__(self, *args, width=width, height=2, command=self.set_listening, **kwargs)
        self.set_char(char)
        self.pack(side=LEFT)
        if not scancode:
            print("Warning: no scancode given for this DlgKeyboardKey")
        self.scancode = scancode
        self.dlg_keyboard = dlg_keyboard
        self.bind("<KeyPress>", self.on_key_press)

    def set_char(self, char):
        if not char:
            char = ""
        self.config(text=char)
        self.update_button_colors(bool(char))

    def set_listening(self):
        self.focus_set()

    def update_button_colors(self, key_set):
        if self['state'] == DISABLED:
            self.configure(bg="#CCD1DA")
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

        self.set_char(char)

        scancode = self.scancode
        if not scancode:
            print("This key was not assigned a scancode and the key: " +
                  char + "can not be saved to the current keyboard layout")
            self.set_char("")
            return

        app = self.dlg_keyboard.app
        layout = app.selected_layout
        if not layout:
            print("Layout is None, not setting key")
            self.set_char("")
            return
        layout.update_key(scancode, char)
        self.tk_focusNext().focus_set()


main()
