// Copyright (c) 2014, Richard Eakin
// Use of this source code (and the Dart VM) is governed by a
// BSD-style license that can be found in the LICENSE.txt file.

#pragma once

#include "cinder/DataSource.h"

#include "include/dart_api.h"

#include <map>

namespace cidart {

typedef std::shared_ptr<class Script>		ScriptRef;
typedef std::map<std::string, Dart_Handle>	DataMap;

typedef std::map<std::string, Dart_NativeFunction> NativeFunctionMap;
typedef std::function<void( const DataMap& )>	ReceiveMapCallback;

// TODO: consider whether to name this Isolate or Script
// - is it at all useful to create an Isolate that doesn't spawn a new script?
// - need to read up more on how Isolates are created in plain dart, and their uses
class Script {
  public:
	struct Options {
		Options& native( const std::string &dartFuncName, Dart_NativeFunction nativeFunc ) { mNativeFunctionMap[dartFuncName] = nativeFunc; return *this; }
		Options& mapReceiver( const ReceiveMapCallback &callback )	{ mReceiveMapCallback = callback; return *this; }

		const NativeFunctionMap&	getNativeFunctionMap() const	{ return mNativeFunctionMap; }
		NativeFunctionMap&			getNativeFunctionMap()			{ return mNativeFunctionMap; }
		const ReceiveMapCallback&	getReceiveMapCallback() const	{ return mReceiveMapCallback; }

	  private:
		NativeFunctionMap			mNativeFunctionMap;
		ReceiveMapCallback			mReceiveMapCallback;
	};

	static ScriptRef	create( const ci::DataSourceRef &source, const Options &options = Options() )	{ return ScriptRef( new Script( source, options ) ); }

	void invoke( const std::string &functionName, int argc = 0, Dart_Handle *args = nullptr );

  private:
	Script( const ci::DataSourceRef &source, const Options &options );

	// Dart_IsolateCreateCallback
	static Dart_Isolate createIsolateCallback( const char* script_uri, const char* main, void* callbackData, char** error );
	// Dart_LibraryTagHandler
	static Dart_Handle libraryTagHandler( Dart_LibraryTag tag, Dart_Handle library, Dart_Handle urlHandle );
	// Dart_NativeEntryResolver
	static Dart_NativeFunction resolveNameHandler( Dart_Handle nameHandle, int numArgs, bool* auto_setup_scope );

	static void printNative( Dart_NativeArguments arguments );
	static void toCinder( Dart_NativeArguments arguments );

	Dart_Isolate				mIsolate;
	ci::fs::path				mMainScriptPath;
	NativeFunctionMap			mNativeFunctionMap;
	ReceiveMapCallback			mReceiveMapCallback;

	std::map<std::string, ci::fs::path>		mImportedLibraries;

	friend class VM;
};

} // namespace ciadart
