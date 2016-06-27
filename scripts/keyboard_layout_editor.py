import os.path
from tkinter import *
from tkinter import filedialog
from xml.etree import ElementTree


def main():
    app = KeyboardLayoutEditor()
    app.mainloop()


class KeyboardLayoutEditor(Tk):
    def __init__(self, *args, **kwargs):
        Tk.__init__(self, *args, **kwargs)

        self.wm_title("Keyboard layouts editor")

        self.minsize(
            width=1000,
            height=270
        )

        self.mainframe = MainFrame(self, app=self)
        self.sidebarframe = SideBarFrame(self.mainframe)
        self.workspaceframe = WorkspaceFrame(self.mainframe)

        # File name of current XML file
        self.file_name = None

        # Element tree of current XML file
        self.tree = None

        self.pack()

    def pack(self):
        self.sidebarframe.pack()
        self.workspaceframe.pack()
        self.mainframe.pack()

    def open_file(self):
        self.file_name = filedialog.askopenfilename(
            filetypes=(("XML Files", ".xml"), ("All Files", "*")),
            title="Choose a keyboard layout XML file"
        )

        if self.file_name == ():
            return

        if os.path.isfile(self.file_name):
            print("Opening file: " + self.file_name)
            with open(self.file_name, 'rt') as f:
                self.tree = ElementTree.parse(f)
        else:
            print("File doesn't exist: " + self.file_name)
            self.file_name = None


class MainFrame(Frame):
    def __init__(self, *args, app=None, **kwargs):
        Frame.__init__(self, *args, **kwargs)

        # Setup main menu bar
        self.menubar = Menu(self)
        menu = Menu(self.menubar, tearoff=0)
        self.menubar.add_cascade(label="File", menu=menu)
        menu.add_command(label="New")
        menu.add_command(label="Open", command=app.open_file)
        self.master.config(menu=self.menubar)


class SideBarFrame(Frame):
    def __init__(self, *args, **kwargs):
        Frame.__init__(self, *args, **kwargs)


class WorkspaceFrame(Frame):
    def __init__(self, *args, **kwargs):
        Frame.__init__(self, *args, **kwargs)


main()
