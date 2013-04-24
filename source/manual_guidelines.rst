.. include:: /shortcuts.rstext

Guidelines for Mixxx Manual writers
***********************************

.. sectionauthor:: S.Brandt <s.brandt@mixxx.org>

What is the intended outcome of the manual?
===========================================
A user who doesn\'t know Mixxx yet should be able to mix two tracks from its
music library in the shortest possible time. Assuming he will be more motivated
to explore the software and get creative.

(Future) characteristics of the Mixxx manual:

**User-friendly**
  Easy to use when, where, and how you need it. Examine, how someone else is
  using the application. Watch someone else use the manual (preferably someone
  who has never seen it before). Be consistent with the instructional design so
  users can follow a set pattern. Don't use the terms you use as a developer,
  try to find the terminology of the user.

**Based on sound learning principles**
  For example users should actually learn from it, not just refer to it. Use
  the KISS principle; keep it sweet and simple. Too much information can be
  overwhelming, so present one concept at a time. Explain simple features in a
  matrix.

**Motivational**
  Keeps users willing to push forward to higher levels. Present general concepts
  first to provide a frame of reference. Then move to more complex topics.

Group problems the user might hit in a particular task right there with the
instruction for that task. Do not force a user to go to a separate
"Troubleshooting" section. We can have such separate sections, but as a author
you should duplicate pitfalls and problems and include a solution in the task.

Technical conventions
=====================

Line Widths
-----------

Please configure your editor to have a max column-width of 80-columns. While it
is not a strict requirement, 80-column cleanliness makes it easy to tile
multiple buffers of code across a laptop screen, which provides significant
efficiency gains to developers.

Screenshots
-----------

Use english language settings when creating screen-shots of the Mixxx interface.
This might change if we ever have true
`i18n <https://en.wikipedia.org/wiki/Internationalization_and_localization>`_.
The preferred file format is PNG. **Don't add shadows** to application window
screen-shots as they are added automatically to the document with style-sheets.

Always include descriptive alt text and a figure description. The latter will be
numbered in the PDF export. That sets them apart from the text below.
Place screen-shots above the context you are going to explain.

Screenshots should only show the necessary area and not the entire screen where
not necessary. Use annotation on the screenshot if necessary to emphasize
elements, use color ``#FF1F90`` if possible for consistency.

.. rst:directive:: figure

   Use this directive to place images like Screen-shots. Example markup: ::

    .. figure:: /_static/icons/mixxx-icon.png
       :width: 64px
       :align: center
       :height: 64px
       :alt: Alternate text on mouse over
       :figclass: pretty-figures

       Insert descriptive caption here

Nice screenshot tools with build-in editor for annotations:

* MacOSX: `Skitch <http://skitch.com/>`_
* Linux: `Shutter <http://shutter-project.org/>`_
* Windows: `PickPic <http://www.picpick.org/download>`_ or
  `Screenshotcaptor <http://www.donationcoder.com/Software/Mouser/screenshotcaptor/>`_

Alternatively, import your screenshots into
`Inkscape <http://www.incscape.org>`_, add annotations and export as .png to
``/static``. Then save the original work as .svg to ``/static`` as well, so
any future contributor can work on your annotations at a later time.

File naming
-----------

As the manual grows over the time with new versions of Mixxx and new screenshots,
it is important to have files named consistently. Save files to the ``/static``
folder or create a sub-folder in there.

::

   Mixxx-<mayor><minor>-<where>-<what>.png

This scheme makes it easy to know which version a screenshot was taken from and
where it belongs and if it must replaced, like e.g.
``Mixxx-111-Preferences-Recording.png``

.. warning:: Do not include any dot in the file names of your screenshots your
             file name or you wont be able to generate PDF with LaTeX.

Admonitions
-----------

The following admonitions are in use:

.. rst:directive:: note

   For anything that should receive a bit more attention. Example markup: ::

      .. note::
         a note

.. rst:directive:: hint

   For supplementary information that lightens the work load. Example markup: ::

      .. hint::
         a helpful hint

.. rst:directive:: seealso

   For references to other documents or websites if they need special attention.
   References to other documents can also be included in the text inline.
   Example markup: ::

      .. seealso::
         a reference and inline link `Google <https://google.com>`_

.. rst:directive:: warning

  Recommended over :rst:dir:`note` for anything that needs to be done with
  caution. Example markup: ::

      .. warning::
         a warning

.. rst:directive:: todo

   Allow inserting todo items into documents and to keep a
   :ref:`automatically generated TODO list <todo-list>` Example markup: ::

      .. todo::
         some task

Substitution
------------

Replacement images or text can be included in the text. They are added through
a substitution (aka alias). This may be appropriate when the replacement image
or text is repeated many times throughout one or more documents, especially if
it may need to change later.

All replacements are kept in the file ``shortcuts.rstext`` which is included at
the beginning of each file in which a substitution is used.

To use an alias for the Mixxx logo, simply put the definition into
``shortcuts.rstext``.

::

   .. |logo| image:: /_static/icons/mixxx-icon.png

Using this image alias, you can insert it easily in the text with ``|logo|`` ,
like this:  |logo|

For a text replacement the code looks similar:

::

   .. |longtext| replace:: Loooooooong text is looooooooong

Using this text alias, you can insert it easily with ``|longtext|`` , like this:
 |longtext| .

.. seealso:: The substitute section in the docs.
             `Here <http://www.thomas-cokelaer.info/tutorials/sphinx/rest_syntax.html#more-about-aliases>`_
             and `also here <http://docutils.sourceforge.net/docs/ref/rst/restructuredtext.html#substitution-definitions>`_

Headings
--------
Normally, there are no heading levels assigned to certain characters as the
structure is determined from the succession of headings. However, for the Python
documentation, this convention is used which you may follow:

   | ``#`` with overline, for parts
   | ``*`` with overline, for chapters
   | ``=`` for sections
   | ``-`` for subsections
   | ``^`` for subsubsections
   | ``"`` for paragraphs

Of course, you are free to use your own marker characters (see the reST
documentation), and use a deeper nesting level, but keep in mind that most
target formats (HTML, LaTeX) have a limited supported nesting depth.

Paragraph-level markup
----------------------

These directives create short paragraphs and can be used inside information
units as well as normal text:

.. rst:directive:: .. versionadded::  version

   This directive documents the version of the project which added the described
   feature. Example markup: ::

      .. versionadded:: 2.5 Add feature description.

.. rst:directive:: .. versionchanged:: version

   Similar to :rst:dir:`versionadded`, but describes when and what changed in
   the named feature in some way (new parameters, changed side effects, etc.).

Other semantic markup
---------------------
The following roles don’t do anything special except formatting the text in a
different style. Nonetheless, use them:

.. rst:role:: guilabel

   Any label used in the interface should be marked with this role, including
   button labels, window titles, field names, menu and menu selection names,
   and even values in selection lists. An accelerator key for the GUI label can
   be included using an ampersand; this will be stripped and displayed
   underlined in the output. To include a literal ampersand, double it. Example
   markup: :guilabel:`&Cancel` ::

     :guilabel:`&Cancel`

.. rst:role:: kbd

   Mark a sequence of keystrokes. Example markup: :kbd:`STRG` + :kbd:`G` ::

     :kbd:`STRG` + :kbd:`G`

.. rst:role:: menuselection

   This is  used to mark a complete sequence of menu selections, including
   selecting submenus and choosing a specific operation. Example markup:
   :menuselection:`Options --> Enable Live Broadcasting` ::

       :menuselection:`Options --> Enable Live Broadcasting`


Short code snippets like terminal commands or paths like ``Mixxx/Recordings``
are highlighted as inline literals using

::

   ``Mixxx/Recordings``

Meta-information markup
-----------------------

.. rst:directive:: .. sectionauthor:: name <email>

   Identifies the author of the current section and helps to keep track of
   contributions. By default, this markup isn't reflected in the output in any
   way. Example markup: ::

      .. sectionauthor:: Jon Doe <name@domain.tld>

Resources
=========

The user manual for Mixxx is written in `reStructuredText (reST)
<http://docutils.sourceforge.net/rst.html/>`_ format using
`Sphinx <http://sphinx.pocoo.org/>`_.

The `Mixxx user manual repostitory <https://code.launchpad.net/~mixxxdevelopers/mixxx/manual>`_
contains the Sphinx source to generate the manual as found at
`<http://mixxx.org/manual/latest/>`_.

Sphinx and RST syntax guides:

* `<http://sphinx.pocoo.org/rest.html>`_
* `<http://www.siafoo.net/help/reST>`_
* `<http://thomas-cokelaer.info/tutorials/sphinx/rest_syntax.html>`_

Steps for use:

#. Install Sphinx (``python-sphinx`` package in Debian/Ubuntu) and GNU make
#. Download Mixxx manual source from
   `launchpad.net <https://code.launchpad.net/~mixxxdevelopers/mixxx/manual>`_
#. Edit .rst files in ``source/``
#. Run ``make html``
#. Open the file ``build/html/index.html`` in your Web browser to view the
   results

.. hint:: Run ``make linkcheck`` in the terminal.
   Sphinx will validate all internal and external links in the document, and let
   you know if any links are malformed, or point to dead URLs.

Editors with Restructured Text (reST) syntax highlighting:

* Mac OSX: `Sublime <http://www.sublimetext.com/2>`_
* Linux: `Kate <http://kate-editor.org/>`_ or
  `Retext <http://sourceforge.net/p/retext/>`_
* Windows: `Sublime <http://www.sublimetext.com/2>`_ or
  `Ìntype <http://inotai.com/intype/>`_

Still not enough? Even more resources:
`<http://stackoverflow.com/questions/2746692/restructuredtext-tool-support>`_


.. todo:: Streamline this document, nobody reads this much isht.
