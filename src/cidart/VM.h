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

class VM {
  public:
	static VM* instance();

	void setCinderDartScriptPath( const ci::fs::path &scriptPath );
	void setCinderDartScriptResource( const ci::DataSourceRef &scriptResource );

	static std::string getVersionString();

  private:
	VM();

	void			loadCinderDartLib();
	std::string		getCinderDartScript();
	const ci::DataSourceRef& getSnapShot();

	std::vector<std::string>	mVMFlags;
	ci::fs::path				mCinderDartScriptPath;
	ci::DataSourceRef			mCinderDartScriptResource; // not used by default, available for windows resources
	ci::DataSourceRef			mSnapshot;

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

	friend class Script;
};

} // namespace cidart
