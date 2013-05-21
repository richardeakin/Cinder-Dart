#pragma once

#include "dart_api.h"

namespace cinderdart {

	struct DartScope {
		DartScope() { Dart_EnterScope(); }
		~DartScope() { Dart_ExitScope(); }
	};

	// TODO: move
	void log(Dart_NativeArguments arguments);


	// TODO: bah, these names are ambigous when used from another file.
	
	Dart_Handle newString(const char* str);

	Dart_Handle newInt( int i );

	std::string getString( Dart_Handle handle );

	int getInt( Dart_Handle handle );

	float getFloat( Dart_Handle handle );

	bool hasFunction( Dart_Handle handle, const std::string &name );

	Dart_Handle callFunction( Dart_Handle target, const std::string &name, int numArgs = 0, Dart_Handle *args = nullptr );

	Dart_Handle getField( Dart_Handle container, const std::string &name );

} // namespace cinderdart