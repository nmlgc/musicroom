# Preservation notes

[`src/`](src/) corresponds to the source archive, [`dist/`](dist/) to the binary
archive. [`libs/`](libs/) contains all used libraries as Git submodules, at the
versions that were originally linked.

The .bgm files and the `valuespit` tool are part of the [bgmlib
repository](libs/bgmlib/).

## How to roughly compile this on Visual Studio 2017

* Make sure you got all submodules:

  ```
  $ git submodule init
  $ git submodule update
  ```

* Follow the instructions in bgmlib's README file.
* Use the provided `libs/vc6libcurl.dsw` solution to build curl. (The Git
  checkout requires automake for auto-generating it.)

* Make sure to consistently set the same *C/C++ → Code Generation → Runtime
  Library* option in all projects, because not all of them will come with the
  same by default.

### Fixes for `musicroom`

* Make sure to consistently set the same *C/C++ → Code Generation → Runtime
  Library* option you also used for the dependent libraries.

* Add `src/tag_base.cpp` to the Visual Studio project.

* Add `FLOAT_MATH_FUNCTIONS` to the list of `#define`'d preprocessor macros at
  *C/C++ → Preprocessor → Preprocessor Definitions*.

  If you want to link curl statically, also `#define CURL_STATICLIB`.

* Point the compiler to the subdirectories of all dependencies by adding the
  following to the list at *C/C++ → General → Additional Include Directories*,
  **in this order**:

  ```
  $(SolutionDir)..\libs\
  $(SolutionDir)..\libs\bgmlib\libs\fox\include\
  $(SolutionDir)..\libs\bgmlib\libs\ogg\include\
  $(SolutionDir)..\libs\bgmlib\libs\vorbis\include\
  $(SolutionDir)..\libs\curl\include\
  ```

* For the libraries, it's easiest to just rename them as expected by the
  project, place them into a single directory, and specify that directory at
  *Linker → General → Additional Library Directories*.

* Place the compiled binary into the `dist/` directory, since it absolutely
  needs `musicroom.cfg` to run.
  Also, copy bgmlib's `bgminfo/` subdirectory there.
