Touhou Project BGM Extractor Readme
===================================

1. About
2. Usage
3. Note about Tasofro games
4. Forcing wiki updates
5. Sources
6. Version History
7. Testimonials

===================================

1. About
--------
This program extracts all the music from the official Windows Touhou games (and a bunch of fangames) and encodes it
into multiple formats via external encoders, while adding the correct* tags for the language of your choice.
You can customize the loop count, fade duration and automatic filenaming.
Updated translation information is automatically downloaded from the Touhou Wiki if selected.
It also can be used as a standalone music room.

A configuration file is provided to change default GUI settings and customize encoding options.

This current version can be downloaded from the following links:
- http://bit.ly/thbgmext
- http://cli.gs/touhou-bgm-extractor
- http://dl.dropbox.com/u/13801415/Touhou/thbgmext.htm
Please only share either one of those links and not the direct link to the current version archive.
This will help to combat mistranslations, especially within the first hours of a new games' release.

If this gets included in any kind of complete Touhou distribution (e.g. a DVD you burn yourself),
or hosted at a more place for newcomers, this tool has achieved its purpose.

* Or at least what I think is correct. If you disagree, please let me know.
--------

2. Usage
--------
Should basically be self-explanatory. However, you may directly drop a supported game's directory or
any file in it onto thbgmext.exe to automatically load it, instead of clicking through the menus.
--------

3. Note about Tasofro games
---------------------------
All Tasofro games except IaMP are storing their music data in Ogg Vorbis format.
Thus, adding loops and fades in any format but FLAC will induce further quality loss.

If you want the original Ogg Vorbis files without reencoding, select OGG, 1 loop and 0 second fades.
Of course this will add tags as well, so it's still superior to just extracting them with Brightmoon.

(You probably won't even notice the quality loss unless you are have high-quality gear or really
sensitive ears, but I still wanted to say it so that no one complains.)
---------------------------

4. Forcing wiki updates
-----------------------
On updating, the program writes the last processed wiki revision to the updated game's BGM file
in order to skip this process if the game is loaded again and the Wiki page wasn't changed,
thus not bugging you with the same updating questions over and over.

However, since I'm also verifying the changes on a regular basis and additionally editing
the BGM files for the official download, you might disagree with my personal opinion,
especially on the track name doubling issue ("Great Fairy Wars ~ Fairy Wars"? Why do we need that twice?).
And because the wiki revision ID gets updated when I'm doing this, you're pretty much stuck with this.

So, to force a wiki update, open a game's BGM file with a text editor and replace the <wikirev> key
in the [update] section with any other value (preferrably 0).

Like this:

[update]
wikirev = 0

Then start the program and load the game again.
-----------------------

5. Sources
----------
We're talking about over 3800 lines of code that were basically typed together,
and they now created a BGM extractor out of this more than 3800 individual pieces of code...
... OK, I know, that isn't funny, and never was.

Source code of the whole application is provided in the ZIP file.
Since this was only a side project to my other programming projects,
there wasn't much thought and even less planning involved in writing this...
But hey, it works, and should be easily expandable, at least now.

If you want to compile it yourself, you'll need the following libraries:

- FOX Toolkit GUI Library (http://fox-toolkit.org/). Go for the development version.

- libogg and libvorbis (http://xiph.org/vorbis/). Required to support Tasofro games.

- cURL (http://curl.haxx.se/). Internet access.

- TagLib (http://developer.k.. no, forget it.)
  I couldn't do anything with the original version, so I changed it to make it actually usable.
  Grab it from here: http://www.mediafire.com/?g91lawdml92c88k

Thanks to the people behind Brightmoon for supplying the sources for decrypting archives of the Tasofro games.
Since I don't have those hacking skills, I couldn't support these games without you.

... ... Hardcoding the tagging code is still a stupid idea. Oh well.
----------

6. Version History
------------------
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
* Initial release, containing music data from TH06 to TH12.8
------------------

7. Testimonials (a.k.a. appreciation of the countless hours I put into creating and maintaining this)
---------------
"I LOVE YOU"
- Nameless Fairy

"Thank you so much for this, sir, you're awesome"
- Nameless Fairy

"I've been looking for something like this for some time."
- Shrinemaiden Forum

"Adorei o programa *-*"
- Santuário Hakurei

"Seems like it supersedes all other utilities made so far."
- Touhou Wiki

"You are awesome and you should feel awesome. Thanks a crapton."
- Drake
---------------