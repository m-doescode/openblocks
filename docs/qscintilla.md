In order to build openblocks on Windows, qscintilla will need to already be installed.

To do this, first download the source archive from [`https://www.riverbankcomputing.com/static/Downloads/QScintilla/2.14.1/QScintilla_src-2.14.1.tar.gz`](https://www.riverbankcomputing.com/static/Downloads/QScintilla/2.14.1/QScintilla_src-2.14.1.tar.gz)

Next, launch the *x64 Native Tools Command Prompt for VS 2022*, and cd into the directory that you extracted the archive to

Now, run `qmake` from your Qt's bin directory to configure it

Once that's done, build and install the project using `nmake install`

The library should now automatically be installed into your Qt installed directory

---

To uninstall the library, run `nmake uninstall`