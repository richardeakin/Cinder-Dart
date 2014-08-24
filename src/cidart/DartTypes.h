// Copyright (c) 2013, Richard Eakin and the Dart project authors.
// Use of this source code (and the Dart VM) is governed by a
// BSD-style license that can be found in the LICENSE.txt file.

#pragma once

#include "include/dart_api.h"
#include "cinder/Color.h"

namespace cidart {

struct DartScope {
	DartScope()		{ Dart_EnterScope(); }
	~DartScope()	{ Dart_ExitScope(); }
};

Dart_Handle newString( const char* str );
Dart_Handle newInt( int i );

bool		isMap( Dart_Handle handle );
bool		isCinderClass( Dart_Handle handle, const char *className );

bool		getBool( Dart_Handle handle );
int			getInt( Dart_Handle handle );
float		getFloat( Dart_Handle handle );
ci::ColorA	getColor( Dart_Handle handle );
std::string getString( Dart_Handle handle );

float		getFloatForKey( Dart_Handle mapHandle, const char *key );

void getValue( Dart_Handle handle, bool *value );
void getValue( Dart_Handle handle, int *value );
void getValue( Dart_Handle handle, size_t *value );
void getValue( Dart_Handle handle, float *value );
void getValue( Dart_Handle handle, ci::Color *value );
void getValue( Dart_Handle handle, ci::ColorA *value );
void getValue( Dart_Handle handle, ci::Vec2i *value );
void getValue( Dart_Handle handle, ci::Vec2f *value );
void getValue( Dart_Handle handle, ci::Vec3i *value );
void getValue( Dart_Handle handle, ci::Vec3f *value );
void getValue( Dart_Handle handle, std::string *value );

bool hasFunction( Dart_Handle handle, const std::string &name );
Dart_Handle callFunction( Dart_Handle target, const std::string &name, int numArgs = 0, Dart_Handle *args = nullptr );

//! Returns a \a Dart_Handle that represents the field \a name on the \a container. If an error occurs, an error handle is returned.
Dart_Handle getField( Dart_Handle container, const std::string &name );
//! Returns the value of type \a T for field \a name on the \a container. If an error occurs, the value returned is default constructed and an error message is printed to console.
template <typename T>
T	getField( Dart_Handle container, const std::string &name )
{
	T result;
	getValue( getField( container, name ), &result );
	return result;
}

std::string getTypeName( Dart_Handle handle );

// Debug utils:
std::string printNativeArgumentsToString( Dart_NativeArguments args, bool printMethodNames = false );

} // namespace cidart
