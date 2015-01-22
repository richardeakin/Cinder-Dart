### Dart runtime libs

This folder contains binaries and header files needed to embed the [dart][dartlang] runtime in a C / C++ app. It is meant to be used with [Cinder-Dart](https://github.com/richardeakin/Cinder-Dart). Below are instructions for how to rebuild binaries for OS X and Windows Desktop.

Here are google's [checkout][checkout] and [build][build] instructions.

### Steps to rebuild dart static binaries for cinder


##### Mac OS X (64-bit):

*this is my personal experience when following the [official instructions][checkout]*

###### Install depot tools:

```
cd <directory where you want the depot_tools to live>
svn co http://src.chromium.org/svn/trunk/tools/depot_tools
export PATH=$PATH:`pwd`//depot_tools
```

###### Full svn checkout:
 
```
mkdir dart-svn-VERSION
cd dart-svn-VERSION
gclient config http://dart.googlecode.com/svn/branches/VERSION/deps/all.deps
gclient sync
```

###### Build:

First, need to set the C++ standard library to libc++. To do this, open up dart/runtime/dart-runtime.xcodeproject, go to the main project settings, and change it from “Compiler Default” to “libc++"

Then, close the Xcode project and run the following (faster than building in Xcode):

```
cd dart
./tools/build.py --arch=x64 runtime
./tools/build.py --arch=x64 --mode=release runtime
```

###### Packaging binaries with libtool:

Currently we need 6 of the static binaries produced by the above build commands, for each configuration. To improve this, the following commands smash these binaries into just one for each config, called 'libdart_runtime.a':

```
cd xcodebuild/DebugX64

libtool -static -o libdart_runtime.a libdart_builtin.a libdart_lib_withcore.a libdart_vm.a libdart_withcore.a libdouble_conversion.a libjscre.a
cp libdart_runtime.a $CINDER_DART_PATH/dart-runtime/lib/macosx/Debug/

cd ../ReleaseX64
// run the same libtool command as above
cp libdart_runtime.a $CINDER_DART_PATH/dart-runtime/lib/macosx/Release/
```

###### To generate the library snapshop binary

Run the followinng from xcodebuild/Debug:

```
./gen_snapshot --snapshot=snapshot_gen.bin
```

##### Windows (64-bit):

https://code.google.com/p/dart/wiki/PreparingYourMachine#Windows

Then install python, also [python windows extensions](http://sourceforge.net/projects/pywin32/files/)

Checkout the dart source following [official directions](https://code.google.com/p/dart/wiki/GettingTheSource). The only thing different I do is I don't checkout 'bleeding_edge' branch, I checkout a specific tagged branch (ex. 1.8)

After checkout, open dart/runtime/dart-runtime.sln and upgrade all projects to vs2013. Close solution, go back to bash terminal and run:

```
./tools/build.py -m debug -a x64 create_sdk
./tools/build.py -m release -a x64 create_sdk
```

###### Packaging binaries with LIB.EXE:

First, you need to get LIB.EXE in your PATH. For me, it was at:

```
C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin
```

Then, run the following from the dart build directories (`${DART_PATH/build/{$BUILD_TARGET}/lib`):

```
LIB.EXE /OUT:libdart_runtime.lib libdart_builtin.lib libdart_lib_withcore.lib libdart_vm.lib libdart_withcore.lib libdouble_conversion.lib libjscre.lib
```

Then copy libdart_runtime.lib to the respective folder (either msw/x64/Release or msw/x64/Debug).

[dartlang]: http://www.dartlang.org/
[checkout]: https://code.google.com/p/dart/wiki/GettingTheSource
[build]: https://code.google.com/p/dart/wiki/Building#Building_everything