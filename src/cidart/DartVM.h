// Copyright (c) 2013, Richard Eakin and the Dart project authors.
// Use of this source code (and the Dart VM) is governed by a
// BSD-style license that can be found in the LICENSE.txt file.

#pragma once

#include "cinder/DataSource.h"
#include "cinder/Function.h"

#include "include/dart_api.h"

#include <memory>
#include <vector>
#include <map>

namespace cidart {

typedef std::shared_ptr<class DartVM>		DartVMRef;
typedef std::map<std::string, Dart_Handle>	DataMap;

class DartVM {
  public:
	static DartVMRef create()	{ return DartVMRef( new DartVM ); }

	typedef std::map<std::string, Dart_NativeFunction> NativeFunctionMap;
	typedef std::function<void( const DataMap& )>	ReceiveMapCallback;

	void loadScript( ci::DataSourceRef script );
	void invoke( const std::string &functionName, int argc = 0, Dart_Handle *args = nullptr );

	void setMapReceiver( const ReceiveMapCallback& callback )	{ mReceiveMapCallback = callback; }

	void addNativeFunction( const std::string dartFuncName, Dart_NativeFunction nativeFunc )	{ mNativeFunctionMap[dartFuncName] = nativeFunc; }

	void setCinderDartScriptPath( const ci::fs::path &scriptPath );
	void setCinderDartScriptResource( const ci::DataSourceRef &scriptResource );

	static std::string getVersionString();

  protected:
	DartVM();

  private:

	void			loadCinderDartLib();
	std::string		getCinderDartScript();

	Dart_Isolate				mIsolate;
	std::vector<std::string>	mVMFlags;
	NativeFunctionMap			mNativeFunctionMap;
	ReceiveMapCallback			mReceiveMapCallback;
	ci::fs::path				mMainScriptPath;
	ci::fs::path				mCinderDartScriptPath;
	ci::DataSourceRef			mCinderDartScriptResource; // not used by default, available for windows resources
	ci::DataSourceRef			mSnapshot;

	std::map<std::string, ci::fs::path>		mImportedLibraries;

	// Dart_IsolateCreateCallback
	static Dart_Isolate createIsolateCallback( const char* script_uri, const char* main, void* data, char** error );
	// Dart_IsolateInterruptCallback
	static bool interruptIsolateCallback();
	// Dart_IsolateUnhandledExceptionCallback
	static void unhandledExceptionCallback( Dart_Handle error );
	// Dart_IsolateShutdownCallback
	static void shutdownIsolateCallback( void *callbackData );
	// Dart_FileOpenCallback
	static void* openFileCallback(const char* name, bool write);
	// Dart_FileReadCallback
	static void readFileCallback(const uint8_t** data, intptr_t* fileLength, void* stream );
	// Dart_FileWriteCallback
	static void writeFileCallback(const void* data, intptr_t length, void* file);
	// Dart_FileCloseCallback
	static void closeFileCallback(void* file);
	// Dart_LibraryTagHandler
	static Dart_Handle libraryTagHandler( Dart_LibraryTag tag, Dart_Handle library, Dart_Handle urlHandle );
	// Dart_NativeEntryResolver
	static Dart_NativeFunction resolveNameHandler( Dart_Handle nameHandle, int numArgs, bool* auto_setup_scope );

	// Native callbacks
	static void printNative( Dart_NativeArguments arguments );
	static void toCinder( Dart_NativeArguments arguments );
};

} // namespace cidart