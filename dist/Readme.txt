Music Room Interface Readme
===========================

1. About
2. Distribution
3. Usage
4. Forcing wiki updates
5. Tag data overview
6. Sources
7. Version History
8. Testimonials

===========================

1. About
--------
This program provides easy playing and extraction of music from the Touhou Project games for Windows.

The music can be encoded into multiple formats (Ogg Vorbis native, FLAC and MP3 via bundled external
encoders), while adding correct tags* in either Japanese or English.
Translations are automatically downloaded from the Touhou Wiki (http://en.touhouwiki.net) if selected.

You can customize the loop count, fade duration and automatic filenaming.

It also can be used as a masstagger for the PC-98 soundtracks from Akyu's Untouched Score
and every single one of ZUN's music CDs.

Version 2 clearly makes this the best Touhou BGM extraction tool ever, right?
This is my way of giving back to the fanbase, and I can't imagine a better way than with this tool.
Especially now.
Oh well.. There is at least one remaining! Stay tuned!

* Or at least what I think is correct. If you disagree, please let me know, or edit the Touhou Wiki.
--------

2. Distribution
---------------
The newest version of this program can be downloaded from the following links:

- http://bit.ly/musicroom_interface
- http://dl.dropbox.com/u/13801415/Touhou/musicroom.htm

Please only share either one of those links and not the direct link to the current version archive.

If this gets included in any kind of complete Touhou distribution (e.g. a DVD you burn yourself
or a complete torrent), this tool has achieved its purpose.

As for the source code, well, do with it whatever you like. It's the internet after all.

Oh yeah, and this program was not made by anybody at shrinemaiden.org.
And especially not by that Drake guy.
If you downloaded it from there, shout at them.
---------------

3. Usage
--------
Should basically be self-explanatory. However, you may directly drop a supported game's directory or
any file in it onto musicroom.exe to automatically load it, instead of clicking through the menus.

This tool supports Vorbis encoded versions of games which normally would have lossless soundtracks.
Of course, if a lossless version is present as well, the program will read from that one.
--------

4. Forcing wiki updates
-----------------------
On updating, the program writes the last processed wiki revision to the updated game's BGM file.
This will skip the update process if the game is loaded again and the Wiki page wasn't changed,
thus not bugging you with the same updating questions over and over.

To force a wiki update, open a game's BGM file with a text editor and replace the <wikirev> key
in the [update] section with any other value (preferrably 0).

Like this:

[update]
wikirev = 0

Then start the program and load the game again.
-----------------------

5. Tag data overview
--------------------

The tagging engine always writes tags in both Japanese and English.
Here, metadata for the selected language is written to the widely and commonly used tag fields
of the format, while metadata for the other language is written to a "custom" tag field,
prefixed with either [EN] or [JP]. Thus, this "additional" data may not be displayed by many players.

The following table shows an overview about the written tag fields for every format.

(Vorbis comments are used in Ogg and FLAC formats, and ID3v2 is used in MP3.
ID3v1 tags are never written. They don't make much sense when working with non-ASCII strings.)

== Game info == 

Vorbis comment      ID3v2
- - - - - - - - - - - - -
ALBUM ARTIST        TXXX/ALBUM ARTIST
ALBUM               TALB
DISCNUMBER          TPOS
TOTALTRACKS         TRCK (format "[track number]/[total tracks]")
DATE                TDRC
GENRE               TCON
CIRCLE              TXXX/CIRCLE
[!LANG]_CIRCLE      TXXX/[!LANG]_CIRCLE
[!LANG]_ARTIST      TXXX/[!LANG]_ARTIST
[!LANG]_ALBUM       TXXX/[!LANG]_ALBUM


== Track info == 

Vorbis comment      ID3v2 
- - - - - - - - - - - - -
ARTIST              TPE1
COMPOSER            TCOM
TITLE               TIT2
TRACKNUMBER         TRCK (format "[track number]/[total tracks]")
COMMENT             COMM
[!LANG]_COMMENT     TXXX/[!LANG]_COMM
[!LANG]_TITLE       TXXX/[!LANG]_TITLE

--------------------

6. Sources
----------
We're talking about over 7500 lines of code that were basically typed together,
and they now created a BGM extractor out of this more than 7500 individual pieces of code...
... OK, I know, that isn't funny, and never was.

Source code of the whole application is provided in the Source.rar file.
The program is split into a back-end library handling reading and writing the .bgm files (bgmlib)
and the front-end GUI handling all the rest.
The valuespit directory contains small command-line tools to quickly create new .bgm files
from the original BGM position data. (e.g. thbgm.fmt for Team Shanghai Alice games)

Project files for Visual C++ 2008 are provided.


If you want to compile the program yourself, you'll need the following additional libraries:

- FOX Toolkit (http://fox-toolkit.org/)
  GUI Library. Go for the development version. Also required for bgmlib!

- DirectX 9 SDK (http://msdn.microsoft.com/en-us/directx/default)
  Optional, but required for song playback.

- libogg and libvorbis (http://xiph.org/)
  NOTE: because of a chaining bug, libvorbis 1.3.2 (Schaufenugget) is the minimum requirement!

- cURL (http://curl.haxx.se/)
  Internet access. Accessed from bgmlib.

Caution: Due to parts of FOX not being thread-safe, bgmlib _always has_ to be linked as static!

Since this was only a side project to my other programming projects,
there wasn't much thought and even less planning involved in writing this...
But hey, it works, and should be easily expandable, at least now.

Thanks to the people behind Brightmoon for supplying the sources for decrypting archives of the Tasofro games.
Since I don't have those hacking skills, I couldn't support these games without you.

... ... Hardcoding the tagging code is still a stupid idea. Oh well.
----------

7. Version History
------------------

- 2011/03/28 (Version 2.0.1)
* Title bar now shows the active game's name and our Akyu icon
* Added tagging support for 「Shuusou Gyoku」 and 「ZUN's Strange Works」
* Fixed slightly wrong starting position for Phantasmagoria of Flower View's おてんば恋娘の冒険
* Fixed wrong looping values for the entirety of 「Mountain of Faith」 and 「Undefined Fantastic Object」's 幽霊客船の時空を越えた旅
* Fixed a crash on game loading

----

- 2011/03/08 (Version 2.0)

*sigh* where should I start.

New features:
* Renamed project to "Music Room Interface", which is a way less bulky name, sounds better, and should still preserve the essence
* Replaced the static game name label with a list box, offering direct game selection, and support for games without directories
* gamedirs.cfg saves those local game directories, providing direct access to the games on subsequent program calls.

* Added support for games to be tag-only, without any extraction...
* ... and used this to add support for the PC-98 game soundtracks from Akyu's Untouched Score and every single CD ZUN ever released

* Banshiryuu (and PBG6 archive) support. If there are other games with music encrypted in PBG6, please let me know!
* Added a nice progress bar used for various time-consuming tasks

* Added support for games compressed with the Touhou Vorbis Compressor (not yet released)

* Included the Ogg Vorbis encoder natively in the program, and added support for chained bitstream output.
  That only works with either lossless or chained bitstream Ogg Vorbis input files, though

* Added a nice encoding settings dialog

Improvements:
* Improved automatic search of the tag update function
* Improved readability of Wiki update text. Should now look OK against every skin ever.
* Updater now automatically resolves Wiki redirects (because the Touhou Wiki guys can't stop moving stuff around -.-)
* Ported streaming engine to DirectSound for enhanced features. Quite nerve-wracking porting work, until I decided to reinvent the wheel.
  Yeah, I wanted to use OpenAL instead, but all this 3D stuff is complete overkill for a project like this. And SDL's too basic, so...
  that basically seals all plans for a native Linux version. But hey, Wine's good enough by now, so nobody should care.
* The BGM and configuration file parser can now add keys not present in the loaded file.
  Also, loading and parsing was speeded up by a factor of 6 (... as if it would seriously matter).
* Moved some information from the readme file into the program, because no one reads readme files after all
* Many other, lesser speed increases in other places. Yeah, I played around a bit for no real reason than to show off better profiling times.
* Status box now replaces game names if language is switched

Technical:
* Split the program into a back-end library (bgmlib) and the front-end GUI part (musicroom)
* Entirely recoded the tagging engine to get rid of the bloat and some bugs of taglib.
  Yes, that was a retarded idea, and one week of wasted work, but now it's less ugly (?),
  and tailor-made for the needs of the international Touhou community and future i18n plans
* Many lesser speed increases in other places. Yeah, I played around a bit for no real reason than to show off better profiling times.
* musicroom.exe now links directly against the native msvcrt.dll. Say goodbye to 1 MB of useless DLLs

----

- 2010/11/16
* The "Yes" option in the Wiki update dialog is actually working now. Seems like I failed logic yet again.
* You may now enter fractional fade durations

- 2010/10/29
* Fixed a fatal bug with IaMP which would hang the system in a very gruesome manner if one tried to load that game
* Fixed high CPU usage in playing mode

- 2010/10/28
* Option to pull updated track info from the Touhou Wiki
* Tag update function, which directly writes track info without re-encoding everything

* Added support for Kioh Gyoku, MegaMari and PatchCon...
* ... and modified some BGM info config keys to manage that
* Added a BGM template file if anyone is interested in adding support for other games I don't have

* More friendly handling of trial versions by testing available tracks
* Extraction and tag update processes run in extra threads now, and you can stop those any time
* Countless GUI improvements and smaller bugs fixed
* We have an official link now, yay~

- 2010/08/16
* Fixed an oversight with the silence removal, which was checked byte- instead of sample-wise (yes, embarrassing, I know).

- 2010/08/14
* Initial release, named "Touhou Project BGM Extractor", containing music data from TH06 to TH12.8
------------------

8. Testimonials (a.k.a. appreciation of the countless hours I put into creating and maintaining this)
---------------
"I LOVE YOU"
- Nameless Fairy

"Thank you so much for this, sir, you're awesome"
- Nameless Fairy

"I've been looking for something like this for some time."
- Shrinemaiden Forum

"Adorei o programa *-*"
- Santuário Hakurei

"Supersedes all other utilities made so far."
- Touhou Wiki
---------------