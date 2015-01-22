## Cinder-Dart

This cinderblock allows you to embed the [Dart](http://www.dartlang.org/) virtual machine in a cinder (C++) app, providing scripting functionality in a modern web language.


#### Installation instructions

Only 64-bit binaries are supplied (you'll have to build for 32-bit if needed).

run the following command:

```
./fetch_libs.sh
```

In any event, or if fmy ghetto script doesn't work, ensure the dart_runtime folder layout is as such (if needed, you can manually download the lib binaries by looking at the package url output from the script):

Mac:
```
dart-runtime
  include
  lib
    macosx
      Debug
        libdart_runtime.a
      Release
        libdart_runtime.a
    snapshot_gen.bin
```

Windows:
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

The api is a combination of the dart [embedder's api](http://dart.googlecode.com/svn/branches/bleeding_edge/dart/runtime/include/dart_api.h) and helper functions in [cidart/Types.h](https://github.com/richardeakin/Cinder-Dart/blob/master/src/cidart/Types.h). Refer to the samples for how to interpret `Dart_Handle`s.

TODO: improve these docs, there are now cidart::getValue<T>( Dart_Handle ) methods for most types.

### Authors

* The Google Dart team
* Richard Eakin (rtepub@gmail.com)
* http://www.libcinder.org
