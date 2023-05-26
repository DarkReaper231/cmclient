OpenTTD README
Last updated:    $LastChangedDate: 2004-07-25 21:22:11 +0200 (Sun, 25 Jul 2004) $
Release version: 0.3.3
------------------------------------------------------------------------


Table of Contents:
------------------
1.0) About
2.0) Contacting
 * 2.1 Reporting Bugs
3.0) Supported Platforms
4.0) Running OpenTTD
5.0) OpenTTD features
6.0) Configuration File
7.0) Compiling
8.0) Translating
 * 8.1 Guidelines
 * 8.2 Translation
 * 8.3 Previewing
X.X) Credits


1.0) About:
---- ------
OpenTTD is a clone of Transport Tycoon Deluxe, a popular game originally
written by Chris Sawyer.  It attempts to mimic the original game as closely
as possible while extending it with new features.


2.0) Contacting:
---- ----------
The easiest way to contact the OpenTTD team is by submitting bug reports or
posting comments in our forums. You can also chat with us on IRC (#openttd
on irc.freenode.net).

The OpenTTD homepage is http://www.openttd.com/.

You can find a forum for OpenTTD at
http://www.tt-forums.net/index.php?c=20


2.1) Reporting Bugs:
---- ---------------
To report a bug, please create a SourceForge account and follow the bugs
link from our homepage. Please make sure the bug is reproducible and
still occurs in the latest daily build or the current SVN version. Also
please look through the existing bug reports briefly to see whether the bug
is not already known.

The SourceForge project page URL is: http://sourceforge.net/projects/openttd/
Click on "Bugs" to see the bug tracker.

Please include the following information in your bug report:
        - OpenTTD version (PLEASE test the latest SVN/daily build)
        - Bug details, including instructions how to reproduce it
        - Platform and compiler (Win32, Linux, FreeBSD, ...)
        - Attach a save game or a screenshot if possible
        - If this bug only occurred recently please note the last
          version without the bug and the first version including
          the bug. That way we can fix it quicker by looking at the
          changes made.


3.0) Supported Platforms:
---- --------------------
OpenTTD has been ported to several platforms and operating systems. It shouldn't
be very difficult to port it to a new platform. The currently working platforms
are:

	Windows - Win32 GDI (faster) or SDL
	Linux - SDL
	FreeBSD - SDL
	MacOS - SDL
	BeOS - SDL
	MorphOS - SDL


4.0) Running OpenTTD:
---- ----------------

Before you run OpenTTD, you need to put the game's datafiles into the data/
subdirectory. You need the following files from the original Windows version
of TTD as OpenTTD makes use of the original TTD artwork.

IMPORTANT! You need the files from the WINDOWS version of TTD.

List of the required files:
sample.cat
trg1r.grf
trgcr.grf
trghr.grf
trgir.grf
trgtr.grf

If you want music you need to copy the gm/ folder from Windows TTD into your OpenTTD folder, not your data folder.

You can change the data path (which contains savegames as well) in Makefile.config by setting DATA_DIR_PREFIX and USE_HOMEDIR.


5.0) OpenTTD features:
---- -----------------

OpenTTD has a lot of features going beyond the original TTD emulation.
Currently there is unfortunately no comprehensive list of features. You could
check the features list on the web, and some optional features can be
controlled through the Configure patches dialog. We also implement some
features known from TTDPatch (http://www.ttdpatch.net/).

Several important non-standard controls:

* Use Ctrl to place presignals
* Ctrl-d toggles double mode on win32


6.0) Configuration File:
---- -------------------
The configuration file for OpenTTD (openttd.cfg) is in a simple windows-like
.INI format. It's mostly undocumented.


7.0) Compiling:
---- ----------
Windows:
  You need Microsoft Visual Studio 6 or .NET. Open the project file
  and it should build automatically. In case you don't build with SDL,
  you need to remove WITH_SDL from the project settings.
  (see also docs/Readme_Windows_MSVC6.0.txt)
  
  You can also build it using the Makefile with MSys/MingW or Cygwin/MingW.
  Please read the Makefile for more information.

Unix:
  OpenTTD can be built either with "make" or with "jam". To build with
  "make", just type "make", or "gmake" on non-gnu systems. To build with "jam",
  first run "./configure" and then "jam". Note that you need SDL to compile
  OpenTTD.
  (see also docs/Readme_Mandrake_Linux.txt)

MacOS:
  Use "make".

BeOS:
  Use "jam".

FreeBSD
  Use "gmake".

MorphOS
  Use "make". Note that you need the MorphOS SDK and the 
  powersdl.library SDK. 

8.0) Translating:
---- -------------------
See http://www.openttd.com/translating.php for up-to-date information.

8.1) Guidelines:
---- -------------------
Here are some translation guidelines which you should follow closely.

    * Please contact the development team before begining the translation process!
      This avoids double work, as someone else may have already started translating to the same language.
    * Translators must use the charater set "Windows latin-1", also known as ISO 8859-1.
      Otherwise, some characters will not display correctly in the game.

8.2) Translation:
---- -------------------
So, now that you've notified the development team about your intention to translate
(You did, right? Of course you did.) you can pick up english.txt (found in the SVN repository
under /lang) and translate.

You must change the first two lines of the file appropriately:

##name English-Name-Of-Language
##ownname Native-Name-Of-Language

Note: Do not alter the following parts of the file:

    * String identifiers (the first word on each line)
    * Parts of the strings which are in curly braces (such as {STRING})
    * Lines beginning with ## (such as ##id), other than the first two lines of the file

8.3) Previewing:
---- -------------------
In order to view the translation in the game, you need to compile your language file
with the strgen utility, which is now bundled with the game.

strgen is a command-line utility. It takes the language filename as parameter.
Example:

strgen lang/german.txt

This results in compiling german.txt and produces another file named german.lng.
Any missing strings are replaced with the english strings. Note that it looks for english.txt
in the lang subdirectory, which is where your language file should also be.

That's all! You should now be able to select the language in the game options.


X.X) Credits:
---- --------
The OpenTTD team:
  Ludvig Strigeus (ludde)        - OpenTTD author, main coder.
  Serge Paquet (vurlix)          - Assistant project manager, coder and webmaster.
  Dominik Scherer (dominik81)    - Coder
  Tamas Farago (Darkvater)       - Coder
  Owen Rudge (orudge)            - Contributor, forum host.
  Bjarni Corfitzen (Bjarni)      - MacOS port
  Cian Duffy (MYOB)              - BeOS port / manual writing
  Christian Rosentreter (tokaiz) - MorphOS/AmigaOS port
  Viktor Strigeus (wiggo)        - Web hosting services
  
Thanks to:
  Bug Reporters - Thanks for all bug reports.
  Josef Drexler - For his great work on TTDPatch.
  Marcin Grzegorczyk - For his TTDPatch work and documentation of TTD internals and graphics.
  Mike Ragsdale - OpenTTD installer
  pasky - Many patches, newgrf support, etc.
  truesatan - Some patches
  TrueLight - Some patches
  Michael Polnick - Some patches
  Michael Blunck - Nice graphics
  George - Canal graphics
  Chris Sawyer - For an amazing game.
  
