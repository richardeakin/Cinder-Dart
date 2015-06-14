// Copyright (c) 2014, Richard Eakin
// Use of this source code (and the Dart VM) is governed by a
// BSD-style license that can be found in the LICENSE.txt file.

#pragma once

#include "cinder/DataSource.h"

#include "include/dart_api.h"

#include <map>

namespace cidart {

typedef std::shared_ptr<class Script>		ScriptRef;

typedef std::function<void( Dart_NativeArguments )>		FunctionCallback;
typedef std::map<std::string, FunctionCallback>			FunctionCallbackMap;
typedef std::map<std::string, Dart_NativeFunction>		NativeCallbackMap;

typedef std::map<std::string, Dart_Handle>				InfoMap;
typedef std::function<void( const InfoMap& )>			ReceiveMapCallback;

//! \brief Class representing a dart script.
//!
//! Notes on error handling and exceptions: When an error occurs before a script is loaded,
//! a cidart::DartException is thrown. If the error occurs in C++ (like in a native callback
//! or parsing some type), you must throw an exception back into dart using Dart_ThrowException.
//! If this goes unhandled, it will cause an error Dart_Handle to be returned from Dart_Invoke(),
//! which will in turn be used to throw a cidart::DartException.
class Script {
  public:
	struct Options {
		Options& native( const std::string &key, Dart_NativeFunction nativeFn );
		Options& native( const std::string &key, const FunctionCallback &callbackFn );
		Options& mapReceiver( const ReceiveMapCallback &callback )	{ mReceiveMapCallback = callback; return *this; }

		const NativeCallbackMap&	getNativeCallbackMap() const	{ return mNativeCallbackMap; }
		NativeCallbackMap&			getNativeCallbackMap()			{ return mNativeCallbackMap; }
		const FunctionCallbackMap&	getFunctionCallbackMap() const	{ return mFunctionCallbackMap; }
		FunctionCallbackMap&		getFunctionCallbackMap()		{ return mFunctionCallbackMap; }
		const ReceiveMapCallback&	getReceiveMapCallback() const	{ return mReceiveMapCallback; }

	  private:
		NativeCallbackMap			mNativeCallbackMap;
		FunctionCallbackMap			mFunctionCallbackMap;
		ReceiveMapCallback			mReceiveMapCallback;
	};

	//! Creates a new Script object from the dart file located at \a sourcePath.
	static ScriptRef	create( const ci::fs::path &sourcePath, const Options &options = Options() )	{ return ScriptRef( new Script( sourcePath, options ) ); }
	//! Creates a new Script object from the dart file located at \a source. \note Only file-based `DataSource`s are supported.
	static ScriptRef	create( const ci::DataSourceRef &source, const Options &options = Options() )	{ return ScriptRef( new Script( source, options ) ); }

	//! Invokes the function \a functionName within the script.
	Dart_Handle invoke( const std::string &functionName, int argc = 0, Dart_Handle *args = nullptr );

	//! Returns a map of libraries this Script imported.
	const std::map<std::string, ci::fs::path>&	getImportedLibraries() const	{ return mImportedLibraries; }

	Dart_Isolate	getIsolate() const	{ return mIsolate; }

  private:
	Script( const ci::fs::path &sourcePath, const Options &options );
	Script( const ci::DataSourceRef &source, const Options &options );

	// Dart_IsolateCreateCallback
	static Dart_Isolate createIsolateCallback( const char *scriptUri, const char *main, const char *packageRoot, void *callbackData, char **error );
	// Dart_LibraryTagHandler
	static Dart_Handle libraryTagHandler( Dart_LibraryTag tag, Dart_Handle library, Dart_Handle urlHandle );
	// Dart_NativeEntryResolver
	static Dart_NativeFunction resolveNameHandler( Dart_Handle nameHandle, int numArgs, bool *autoSetupScope );

	// Dart_NativeFunction's
	static void callNativeFunctionHandler( Dart_NativeArguments args ); // used to call std::function's
	static void printNativeHandler( Dart_NativeArguments args );		// handles print() statements
	static void toCinderHandler( Dart_NativeArguments args );			// toCinder() calls
	static void getWindowSize( Dart_NativeArguments args );
	static void getWindowWidth( Dart_NativeArguments args );
	static void getWindowHeight( Dart_NativeArguments args );
	static void getElapsedSeconds( Dart_NativeArguments args );
	static void getElapsedFrames( Dart_NativeArguments args );

	void			init();
	std::string		loadSourceImpl( const ci::fs::path &sourcePath );
	ci::fs::path	resolveRelativeImportPath( const std::string &url );

	Dart_Isolate				mIsolate;
	ci::fs::path				mMainScriptPath;
	NativeCallbackMap			mNativeCallbackMap;
	FunctionCallbackMap			mFunctionCallbackMap;
	ReceiveMapCallback			mReceiveMapCallback;

	std::map<std::string, ci::fs::path>		mImportedLibraries;

	friend class VM;
};

} // namespace ciadart
