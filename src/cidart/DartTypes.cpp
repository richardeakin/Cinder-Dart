// Copyright (c) 2013, Richard Eakin and the Dart project authors.
// Use of this source code (and the Dart itself) is governed by a
// BSD-style license that can be found in the LICENSE.txt file.

#include "cidart/DartTypes.h"
#include "debug.h"

using namespace std;

namespace cidart {

	void console( Dart_NativeArguments arguments ) {
		DartScope enterScope;
		Dart_Handle handle = Dart_GetNativeArgument( arguments, 0 );
		CHECK_DART( handle );

		LOG_V << getString( handle ) << std::endl;
	}

	Dart_Handle newString( const char* str ) {
		return Dart_NewStringFromCString(str);
	}

	Dart_Handle newInt( int i ) {
		return Dart_NewInteger( i );
	}

	string getString( Dart_Handle handle ) {
		const char *result;
		CHECK_DART( Dart_StringToCString( handle, &result ) );
		return string( result );
	}

	// TODO: if type isn't int but it is number, get it however possible and cast it to int
	int getInt( Dart_Handle handle ) {
		int64_t result;
		CHECK_DART( Dart_IntegerToInt64( handle, &result ) );
		return static_cast<int>( result );
	}

	float getFloat( Dart_Handle handle ) {
		double result;
		CHECK_DART( Dart_DoubleValue( handle, &result ) );
		return static_cast<float>( result );
	}

	bool hasFunction( Dart_Handle handle, const string &name ) {
		Dart_Handle result = Dart_LookupFunction( handle, newString( name.c_str() ) );
		CHECK_DART( result );
		return ! Dart_IsNull( result );
	}

	Dart_Handle callFunction( Dart_Handle target, const string &name, int numArgs, Dart_Handle *args ) {
		Dart_Handle result = Dart_Invoke( target, newString( name.c_str() ), numArgs, args );
		CHECK_DART( result );
		return result;
	}

	Dart_Handle getField( Dart_Handle container, const string &name ) {
		Dart_Handle result = Dart_GetField( container,  newString( name.c_str() ) );
		CHECK_DART( result );
		return result;
	}

	string getClassName( Dart_Handle handle ) {
		Dart_Handle instanceClass = Dart_InstanceGetClass( handle );
		CHECK_DART( instanceClass );
		Dart_Handle className = Dart_ClassName( instanceClass );
		CHECK_DART( className );

		return getString( className );
	}

	bool isMap( Dart_Handle handle ) {
		Dart_Handle instanceClass = Dart_InstanceGetClass( handle );
		return hasFunction( instanceClass, "keys" ); // all map classes contain this function
	}

//	bool isMap( Dart_Handle handle ) {
//		Dart_Handle coreUrl = newString( "dart:core" );
//		CHECK_DART( coreUrl );
//		Dart_Handle coreLib = Dart_LookupLibrary( coreUrl );
//		CHECK_DART( coreLib );
//
//		Dart_Handle mapClassName = newString( "Map" );
//		CHECK_DART( mapClassName );
//		Dart_Handle mapClass = Dart_GetClass( coreLib, mapClassName );
//		CHECK_DART( mapClass );
//
//		// FIXME: this fails assertion with the following:
//		// /Volumes/ssd/code/chromium/dart-svn/dart/runtime/vm/object.cc:9653: error: expected: !type_class.HasTypeArguments()
//		bool result;
//		CHECK_DART( Dart_ObjectIsType( handle, mapClass, &result ) );
//		return result;
//	}

	bool isColor( Dart_Handle handle ) {
		Dart_Handle cinderLib = Dart_LookupLibrary( newString( "cinder" ) );
		CHECK_DART( cinderLib );
		Dart_Handle colorClass = Dart_GetClass( cinderLib, newString( "Color" ) );
		CHECK_DART( colorClass );

		bool result;
		CHECK_DART( Dart_ObjectIsType( handle, colorClass, &result ) );
		return result;
	}

	ci::ColorA getColor( Dart_Handle handle ) {
		ci::ColorA result;
		if( ! isColor( handle ) ) {
			LOG_E << "expected handle to be of type Color" << endl;
			return result;
		}

		result.r = getFloat( getField( handle, "r" ) );
		result.g = getFloat( getField( handle, "g" ) );
		result.b = getFloat( getField( handle, "b" ) );
		result.a = getFloat( getField( handle, "a" ) );

		return result;
	}

} // namespace cidart