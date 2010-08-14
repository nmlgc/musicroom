# Preservation notes

[`src/`](src/) corresponds to the source archive, [`dist/`](dist/) to the binary
archive. [`libs/`](libs/) contains all used libraries as Git submodules, at the
versions that were originally linked.

In order to keep the diffs coherent, the .bgm files and the `valuespit` tool
were already moved to the [bgmlib repository](libs/bgmlib/), despite bgmlib not
having existed as a separate library before 2.0.

## How to roughly compile this on Visual Studio 2017

* Make sure you got all submodules:

  ```
  $ git submodule init
  $ git submodule update
  ```

* Use CMake to generate a Visual Studio x86 (non-Win64) solution for taglib.
  Make sure you've selected the `ENABLE_STATIC` option before generating.
* Convert all other Visual Studio solutions to the current version. With FOX,
  you only need to convert `fox.dsp` and `reswrap.dsp`, you can disable all
  other projects to save time.
* Make sure to consistently set the same *C/C++ → Code Generation → Runtime
  Library* option in all projects, because not all of them will come with the
  same by default.

  Most of them should then compile fine, except…

### Fixes for the FOX Toolkit

* Open `libs\fox\windows\vcpp\fox\fox.vcxproj` in a plaintext editor and remove
  the following file names from the two `reswrap` command lines (simply Ctrl-F
  for `reswrap`):
  * `bigfloppy3.gif`
  * `bigfloppy5.gif`
  * `bigharddisk.gif`
  * `minifloppy3.gif`
  * `minifloppy5.gif`

  Or, simply replace everything matching the regular expression

  ```regex
  (bigfloppy3.gif|bigfloppy5.gif|bigharddisk.gif|minifloppy3.gif|minifloppy5.gif)
  ```

  with an empty string.
* Remove the nonexistent `FXReactor.cpp` from the `fox` project.
* Set *C/C++ → Code Generation → Enable Function-Level Linking* to **Yes (/Gy)**
  for the Debug configuration of both the `reswrap` and `fox` projects.
* Add `HAVE_STRTOLL` and `HAVE_STRTOULL` to the list of `#define`'d preprocessor
  macros for the `fox` project at *C/C++ → Preprocessor → Preprocessor
  Definitions*.
* You will later run into problems with FXAtomic and the Interlocked family of
  functions. This should work around those:

```diff
diff --git a/include/FXAtomic.h b/include/FXAtomic.h
index 31d89b34..dd4c22ce 100644
--- a/include/FXAtomic.h
+++ b/include/FXAtomic.h
@@ -21,6 +21,47 @@
 #ifndef FXATOMIC_H
 #define FXATOMIC_H

+#ifdef WIN32
+typedef long LONG;
+
+LONG InterlockedExchange(LONG volatile *, LONG);
+LONG InterlockedExchangeAdd(LONG volatile *, LONG);
+LONG InterlockedCompareExchange(LONG volatile *, LONG, LONG);
+LONG InterlockedAdd(LONG volatile *, LONG);
+LONG InterlockedOr(LONG volatile *, LONG);
+LONG InterlockedAnd(LONG volatile *, LONG);
+LONG InterlockedXor(LONG volatile *, LONG);
+
+#endif
+

 namespace FX {
```

### Fixes for the BGM extractor itself (`thbgmext.sln`)

* Make sure to consistently set the same *C/C++ → Code Generation → Runtime
  Library* option you also used for the dependent libraries.

* Add `FLOAT_MATH_FUNCTIONS` and `TAGLIB_STATIC` to the list of `#define`'d
  preprocessor macros at *C/C++ → Preprocessor → Preprocessor Definitions*.

* Point the compiler to the subdirectories of all dependencies by adding the
  following to the list at *C/C++ → General → Additional Include Directories*,
  **in this order**:

  ```
  $(SolutionDir)..\libs\fox\include\
  $(SolutionDir)..\libs\ogg\include\
  $(SolutionDir)..\libs\vorbis\include\
  $(SolutionDir)..\libs\taglib_mod\taglib\
  $(SolutionDir)..\libs\taglib_mod\taglib\toolkit\
  $(SolutionDir)..\libs\taglib_mod\taglib\ogg\
  ```

  Plus the directory in which CMake wrote the taglib solution.

* For the libraries, it's easiest to just rename them as expected by the
  project, place them into a single directory, and specify that directory at
  *Linker → General → Additional Library Directories*, replacing that absolute
  path there.

* The Debug build tries to link `zlibd.lib` for some reason, but it's actually
  not needed, so just remove it from the link dependency list at *Linker → Input
  → Additional Dependencies*.

* Place the compiled binary into the `dist/` directory, since it absolutely
  needs `thbgmext.cfg` to run.
  Also, copy the `.bgm` files from bgmlib's `bgminfo/` subdirectory there.
