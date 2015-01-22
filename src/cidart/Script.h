// Copyright (c) 2014, Richard Eakin
// Use of this source code (and the Dart VM) is governed by a
// BSD-style license that can be found in the LICENSE.txt file.

#pragma once

#include "cinder/DataSource.h"

#include "include/dart_api.h"

#include <map>

namespace cidart {

typedef std::shared_ptr<class Script>		ScriptRef;

typedef std::function<void( Dart_NativeArguments )>		NativeCallback;
typedef std::map<std::string, NativeCallback>			NativeCallbackMap;

typedef std::map<std::string, Dart_Handle>	DataMap;
typedef std::function<void( const DataMap& )>			ReceiveMapCallback;

//! Class representing a dart script.
class Script {
  public:
	struct Options {
		Options& native( const std::string &dartFuncName, const NativeCallback &nativeFunc )	{ mNativeCallbackMap[dartFuncName] = nativeFunc; return *this; }
		Options& mapReceiver( const ReceiveMapCallback &callback )	{ mReceiveMapCallback = callback; return *this; }

		const NativeCallbackMap&	getNativeCallbackMap() const	{ return mNativeCallbackMap; }
		NativeCallbackMap&			getNativeCallbackMap()			{ return mNativeCallbackMap; }
		const ReceiveMapCallback&	getReceiveMapCallback() const	{ return mReceiveMapCallback; }

	  private:
		NativeCallbackMap			mNativeCallbackMap;
		ReceiveMapCallback			mReceiveMapCallback;
	};

	//! Creates a new Script object from the dart file located at \a sourcePath.
	static ScriptRef	create( const ci::fs::path &sourcePath, const Options &options = Options() )	{ return ScriptRef( new Script( sourcePath, options ) ); }
	//! Creates a new Script object from the dart file located at \a source. \note Only file-based `DataSource`s are supported.
	static ScriptRef	create( const ci::DataSourceRef &source, const Options &options = Options() )	{ return ScriptRef( new Script( source, options ) ); }

	void invoke( const std::string &functionName, int argc = 0, Dart_Handle *args = nullptr );

  private:
	Script( const ci::fs::path &sourcePath, const Options &options );
	Script( const ci::DataSourceRef &source, const Options &options );

	// Dart_IsolateCreateCallback
	static Dart_Isolate createIsolateCallback( const char *scriptUri, const char *main, const char *packageRoot, void *callbackData, char **error );
	// Dart_LibraryTagHandler
	static Dart_Handle libraryTagHandler( Dart_LibraryTag tag, Dart_Handle library, Dart_Handle urlHandle );
	// Dart_NativeEntryResolver
	static Dart_NativeFunction resolveNameHandler( Dart_Handle nameHandle, int numArgs, bool *autoSetupScope );

	// Dart_NativeFunction - this is used for all callbacks, so we can use std::function's instead of c function pointers
	static void nativeCallbackHandler( Dart_NativeArguments args );

	static void printNative( Dart_NativeArguments arguments );
	void toCinder( Dart_NativeArguments arguments );

	void			init();
	std::string		loadSourceImpl( const ci::fs::path &sourcePath );

	Dart_Isolate				mIsolate;
	ci::fs::path				mMainScriptPath;
	NativeCallbackMap			mNativeCallbackMap;
	ReceiveMapCallback			mReceiveMapCallback;
	std::string					mLatestNativeCallbackName;

	std::map<std::string, ci::fs::path>		mImportedLibraries;

	friend class VM;
};

} // namespace ciadart
