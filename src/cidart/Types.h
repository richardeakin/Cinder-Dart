// Copyright (c) 2013, Richard Eakin and the Dart project authors.
// Use of this source code (and the Dart VM) is governed by a
// BSD-style license that can be found in the LICENSE.txt file.

#pragma once

#include "include/dart_api.h"

#include "cinder/Vector.h"
#include "cinder/Quaternion.h"
#include "cinder/Color.h"
#include "cinder/Rect.h"
#include "cinder/Exception.h"
#include "cinder/Log.h"

namespace cidart {

struct DartScope {
	DartScope()		{ Dart_EnterScope(); }
	~DartScope()	{ Dart_ExitScope(); }
};

Dart_Handle toDart( const char *str );
Dart_Handle toDart( const std::string &str );
Dart_Handle toDart( int value );
Dart_Handle toDart( float value );
Dart_Handle toDart( double value );
Dart_Handle toDart( const ci::vec3 &value );

bool		isCinderClass( Dart_Handle handle, const char *className );

// TODO: remove, replace with getMapValueForKey<T>()
float		getFloatForKey( Dart_Handle mapHandle, const char *key );

void getValue( Dart_Handle handle, bool *value );
void getValue( Dart_Handle handle, int *value );
void getValue( Dart_Handle handle, size_t *value );
void getValue( Dart_Handle handle, float *value );
void getValue( Dart_Handle handle, double *value );
void getValue( Dart_Handle handle, ci::Color *value );
void getValue( Dart_Handle handle, ci::ColorA *value );
void getValue( Dart_Handle handle, ci::ivec2 *value );
void getValue( Dart_Handle handle, ci::vec2 *value );
void getValue( Dart_Handle handle, ci::dvec2 *value );
void getValue( Dart_Handle handle, ci::ivec3 *value );
void getValue( Dart_Handle handle, ci::vec3 *value );
void getValue( Dart_Handle handle, ci::dvec3 *value );
void getValue( Dart_Handle handle, ci::ivec4 *value );
void getValue( Dart_Handle handle, ci::vec4 *value );
void getValue( Dart_Handle handle, ci::dvec4 *value );
void getValue( Dart_Handle handle, ci::mat4 *value );
void getValue( Dart_Handle handle, ci::quat *value );
void getValue( Dart_Handle handle, ci::dquat *value );
void getValue( Dart_Handle handle, ci::Rectf *value );
void getValue( Dart_Handle handle, ci::log::Level *value );
void getValue( Dart_Handle handle, std::string *value );

//! Returns the value of type \a T held by \a handle. If an error occurs, the value returned is default constructed and an error message is logged.
template <typename T>
T	getValue( Dart_Handle handle )
{
	T result;
	getValue( handle, &result );
	return result;
}

//! Returns a \a Dart_Handle that represents the field \a name on \a container.
Dart_Handle getField( Dart_Handle container, const std::string &name );
//! Returns a \a Dart_Handle that represents the field \a name on \a container.
Dart_Handle getField( Dart_Handle container, const char *name );

//! Returns the value of type \a T for field \a name on the \a container. If an error occurs, the value returned is default constructed and an error message is logged.
template <typename T>
T	getField( Dart_Handle container, const std::string &name )
{
	T result;
	getValue( getField( container, name ), &result );
	return result;
}

//! Returns the value of type \a T for field \a name on the \a container, or \a defaultValue if the field was null or an error occurs.
template <typename T>
T	getFieldOrDefault( Dart_Handle container, const std::string &name, const T &defaultValue )
{
	Dart_Handle fieldHandle = cidart::getField( container, name );
	if( Dart_IsNull( fieldHandle ) )
		return defaultValue;
	else
		return cidart::getValue<T>( fieldHandle );
}

Dart_Handle getMapValueForKey( Dart_Handle mapHandle, const char *key );

template <typename T>
T getMapValueForKey( Dart_Handle mapHandle, const char *key )
{
	CI_ASSERT( Dart_IsMap( mapHandle ) );
	Dart_Handle valueHandle = Dart_MapGetAt( mapHandle, toDart( key ) );

	T result;
	getValue( valueHandle, &result );
	return result;
}

//! Returns the native argument of type \a T at \a index. If an error occurs, the value returned is default constructed and an error message is logged.
template <typename T>
T	getArg( Dart_NativeArguments args, int index )
{
	T result;
	Dart_Handle argHandle = Dart_GetNativeArgument( args, index );

	getValue( argHandle, &result );
	return result;
}

bool			hasFunction( Dart_Handle handle, const std::string &name );
Dart_Handle		callFunction( Dart_Handle target, const std::string &name, int numArgs = 0, Dart_Handle *args = nullptr );

std::string		getTypeName( Dart_Handle handle );

// Debug utils:
std::string		printNativeArgumentsToString( Dart_NativeArguments args, bool printMethodNames = false );

} // namespace cidart
