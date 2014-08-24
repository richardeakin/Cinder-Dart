## Cinder-Dart

This is an experimental cinderblock that embeds the [Dart](http://www.dartlang.org/) virtual machine in a cinder (C++) app, providing scripting functionality in a modern web language.


#### Installation instructions

Cinder is expected to be found at './cinder', I prefer to symlink my copy there.

Binaries for OS X are included via submodule reference (warning: they are big, ~100mb). To checkout everything in one go, do:

```
git clone --recursive git://github.com/richardeakin/Cinder-Dart.git $CINDER_PATH/blocks/dart
```

### Usage

You currently need to get your hands dirty with the [embedder's api](http://dart.googlecode.com/svn/branches/bleeding_edge/dart/runtime/include/dart_api.h). See [DartBasic](test/DartBasic/src/DartBasicApp.cpp) for an example of how `Dart_Handle`'s are currently exposed the the app.


### Authors

Thanks for Dart, google!

Richard Eakin (rtepub@gmail.com)

http://www.libcinder.org
