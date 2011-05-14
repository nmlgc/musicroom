Music Room Interface
Source distribution readme
==========================

NOTE: This was only a side project to my other programming projects, and
      there wasn't much thought and even less planning involved in writing this.
      Hence, this code is, without question, terrible, and should not serve as a model.

We're talking about over 7500 lines of code that were basically typed together,
and they now created a BGM extractor out of this more than 7500 individual pieces of code...
... OK, I know, that isn't funny, and never was.

The program is split into a back-end library handling reading and writing the .bgm files (bgmlib)
and the front-end GUI handling all the rest.
The valuespit directory contains small command-line tools to quickly create new .bgm files
from the original BGM position data. (e.g. thbgm.fmt for Team Shanghai Alice games)

Project files for Visual C++ 2008 are provided.

If you want to compile the program yourself, you'll need the following additional libraries:

- bgmlib = 2.2
Library to access *.bgm files containing track information.

bgmlib depends on:
- - - - - - - - - 

    - FOX-Toolkit >= 1.7.22 (http://fox-toolkit.org/)
      GUI Library
	
  - libvorbis >= 1.3.2 (http://www.xiph.org/vorbis/)
    CAUTION: Because of a chaining bug in previous versions,
	version 1.3.2 (Schaufenugget) is indeed the minimum requirement!
	
  - libogg >= 1.2.1 (http://www.xiph.org/ogg/)
  
- - - - - - - - -   

- th_tool_shared = 2.2 
  Random collection of GUI functions shared across my Touhou BGM projects.
 
- DirectX >= 9.0 (http://msdn.microsoft.com/en-us/directx/default)
  Optional. Only needed for song playback. 	 	
  
- libcurl >= 7.21.1 (http://curl.haxx.se/)
  Internet access. Required for pulling track info from the Touhou Wiki. 	 	

Caution: Due to parts of FOX apparently not being thread-safe, bgmlib _always has_ to be linked as static!

Thanks to the people behind Brightmoon for supplying the sources for decrypting archives of the Tasofro games.
Since I don't have those hacking skills, I couldn't support these games without you.

... ... Hardcoding the tagging code is still a stupid idea. Oh well.