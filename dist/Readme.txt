Touhou Project BGM Extractor
----------------------------

This program extracts all the music from the Windows Touhou games and encodes it into multiple formats
via external encoders, while adding the correct* tags for the language of your choice.
You can customize the loop count, fade duration and automatic filenaming.
It also can be used as a standalone music room.

Feel free to rehost this anywhere you like.
If this gets included in any kind of complete Touhou distribution (e.g. a DVD you burn yourself),
or hosted at a more visible place for newcomers, this tool has achieved its purpose.

* Or at least what I think is correct. If you disagree, please let me know.
  Just post something on Pooshlmer, I'll notice.

----------------------------

Sources
-------

Source code of the whole application is provided in the ZIP file.
Since this was only a side project to my other programming projects,
there wasn't much thought and even less planning involved in writing this...
But hey, it works, and should be easily expandable, at least now.

If you want to compile it yourself, you'll need the following libraries:

- FOX Toolkit GUI Library (http://fox-toolkit.org/). Go for the development version.
- TagLib (http://developer.k.. no, forget it.)
  I couldn't do anything with the original version, so I changed it to make it actually usable.
  Grab it from here:

Thanks to the people behind Brightmoon for supplying the sources for decrypting archives of the Tasofro games.
Since I don't have those hacking skills, I couldn't support these games without you.

... ... Hardcoding the tagging code is still a stupid idea. Oh well.

-------

Version History
---------------
- 14.08.2010
Initial release, containing music data from TH06 to TH12.8