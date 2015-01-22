// Copyright (c) 2013, Richard Eakin and the Dart project authors.
// Use of this source code (and the Dart VM) is governed by a
// BSD-style license that can be found in the LICENSE.txt file.

#pragma once

#include "include/dart_api.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/Rect.h"
#include "cinder/Exception.h"

#if( CINDER_VERSION < 900 )

// for compatibility with 0.8.6, add typedef's for used vectypes back to old ci vectypes
namespace cinder {
	typedef Vec2f	vec2;
	typedef Vec2i	ivec2;
	typedef Vec2d	dvec2;
	typedef Vec3f	vec3;
	typedef Vec3i	ivec3;
	typedef Vec3d	dvec3;
}

#endif

namespace cidart {

struct DartScope {
	DartScope()		{ Dart_EnterScope(); }
	~DartScope()	{ Dart_ExitScope(); }
};

Dart_Handle toDart( const char *str );
Dart_Handle toDart( const std::string &str );

bool		isCinderClass( Dart_Handle handle, const char *className );

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
void getValue( Dart_Handle handle, ci::Rectf *value );
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

class DartException : public ci::Exception {
  public:
	DartException( const std::string &descr ) : mDescription( descr )	{}
	virtual const char* what() const throw()	{ return mDescription.c_str(); }
  protected:
	std::string mDescription;
};

} // namespace cidart
