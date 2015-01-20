## Cinder-Dart

This cinderblock allows you to embed the [Dart](http://www.dartlang.org/) virtual machine in a cinder (C++) app, providing scripting functionality in a modern web language.


#### Installation instructions

Only 64-bit binaries are supplied (you'll have to build for 32-bit if needed).

##### Mac OS X
run the following command:

```
./fetch_libs_mac.sh
```

##### Windows Desktop

Download the compiled static libraries from [this link](https://dl.dropboxusercontent.com/u/3905723/cinder/dart-runtime-packages/dart_runtime_libs_1.8.5_msw_x64.zip)

Unpack the contents to the dart_runtime folder. You should end up with the following structure:

```
dart-runtime
  include
  lib
    msw
      x64
        Debug
          libdart_runtime.lib
        Release
          libdart_runtime.lib
  snapshot_gen.bin
```

### Usage

You currently need to get your hands dirty with the [embedder's api](http://dart.googlecode.com/svn/branches/bleeding_edge/dart/runtime/include/dart_api.h). See [DartBasic](test/DartBasic/src/DartBasicApp.cpp) for an example of how `Dart_Handle`'s are currently exposed the the app.

### Authors

* The Google Dart team
* Richard Eakin (rtepub@gmail.com)
* http://www.libcinder.org
