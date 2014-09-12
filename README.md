## Cinder-Dart

This cinderblock allows you to embed the [Dart](http://www.dartlang.org/) virtual machine in a cinder (C++) app, providing scripting functionality in a modern web language.


#### Installation instructions

64-bit binaries for OS X should be downloaded with the following command:

```
./fetch_libs.sh
```

### Usage

You currently need to get your hands dirty with the [embedder's api](http://dart.googlecode.com/svn/branches/bleeding_edge/dart/runtime/include/dart_api.h). See [DartBasic](test/DartBasic/src/DartBasicApp.cpp) for an example of how `Dart_Handle`'s are currently exposed the the app.

### Authors

* The Google Dart team
* Richard Eakin (rtepub@gmail.com)
* http://www.libcinder.org
