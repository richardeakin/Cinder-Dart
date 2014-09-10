// Copyright (c) 2013, Richard Eakin and the Dart project authors.
// Use of this source code (and the Dart VM) is governed by a
// BSD-style license that can be found in the LICENSE.txt file.

#include "cidart/DartTypes.h"
#include "cidart/DartDebug.h"

#include "dart_mirrors_api.h"

#include <sstream>

using namespace std;

namespace cidart {

namespace {

template<typename T>
void getNumberValueImpl( Dart_Handle handle, T *value )
{
	if( Dart_IsDouble( handle ) ) {
		double result;

		CIDART_CHECK( Dart_DoubleValue( handle, &result ) );
		*value = static_cast<T>( result );
	}
	else if( Dart_IsInteger( handle ) ) {
		int64_t result;
		CIDART_CHECK( Dart_IntegerToInt64( handle, &result ) );
		*value = static_cast<T>( result );
	}
	else
		CI_LOG_E( "expected handle to be either of type float or int" );
}
	
} // anonymous namespace

Dart_Handle toDart( const char *str )
{
	return Dart_NewStringFromCString( str );
}

Dart_Handle toDart( const std::string &str )
{
	return toDart( str.c_str() );
}

string toString( Dart_Handle handle )
{
	const char *result;
	CIDART_CHECK( Dart_StringToCString( handle, &result ) );
	return string( result );
}

float getFloatForKey( Dart_Handle mapHandle, const char *key )
{
	CI_ASSERT( isMap( mapHandle ) );

	Dart_Handle args[] = { cidart::toDart( key ) };
	Dart_Handle valueHandle = cidart::callFunction( mapHandle, "[]", 1, args );
	CIDART_CHECK( valueHandle );

	return cidart::getValue<float>( valueHandle );
}

void getValue( Dart_Handle handle, bool *value )
{
	CIDART_CHECK( Dart_BooleanValue( handle, value ) );
}

void getValue( Dart_Handle handle, int *value )
{
	getNumberValueImpl( handle, value );
}

void getValue( Dart_Handle handle, size_t *value )
{
	getNumberValueImpl( handle, value );
}

void getValue( Dart_Handle handle, float *value )
{
	getNumberValueImpl( handle, value );
}

void getValue( Dart_Handle handle, ci::Color *value )
{
	Dart_Handle r = getField( handle, "r" );
	Dart_Handle g = getField( handle, "g" );
	Dart_Handle b = getField( handle, "b" );

	if( Dart_IsError( r ) || Dart_IsError( g ) || Dart_IsError( b ) ) {
		CI_LOG_E( "expected handle to have fields 'r', 'g' and 'b'" );
		return;
	}

	value->r = getValue<float>( r );
	value->g = getValue<float>( g );
	value->b = getValue<float>( b );
}

void getValue( Dart_Handle handle, ci::ColorA *value )
{
	Dart_Handle r = getField( handle, "r" );
	Dart_Handle g = getField( handle, "g" );
	Dart_Handle b = getField( handle, "b" );
	Dart_Handle a = getField( handle, "a" );

	if( Dart_IsError( r ) || Dart_IsError( g ) || Dart_IsError( b ) || Dart_IsError( a ) ) {
		CI_LOG_E( "expected handle to have fields 'r', 'g', 'b' and 'a'" );
		return;
	}

	value->r = getValue<float>( r );
	value->g = getValue<float>( g );
	value->b = getValue<float>( b );
	value->a = getValue<float>( a );
}

void getValue( Dart_Handle handle, ci::ivec2 *value )
{
	Dart_Handle x = getField( handle, "x" );
	Dart_Handle y = getField( handle, "y" );

	if( Dart_IsError( x ) || Dart_IsError( y ) ) {
		CI_LOG_E( "expected handle to have fields 'x' and 'y'" );
		return;
	}

	value->x = getValue<int>( x );
	value->y = getValue<int>( y );
}

void getValue( Dart_Handle handle, ci::vec2 *value )
{
	Dart_Handle x = getField( handle, "x" );
	Dart_Handle y = getField( handle, "y" );

	if( Dart_IsError( x ) || Dart_IsError( y ) ) {
		CI_LOG_E( "expected handle to have fields 'x' and 'y'" );
		return;
	}

	value->x = getValue<float>( x );
	value->y = getValue<float>( y );
}

void getValue( Dart_Handle handle, ci::ivec3 *value )
{
	Dart_Handle x = getField( handle, "x" );
	Dart_Handle y = getField( handle, "y" );
	Dart_Handle z = getField( handle, "z" );

	if( Dart_IsError( x ) || Dart_IsError( y ) ) {
		CI_LOG_E( "expected handle to have fields 'x', 'y', and 'z'" );
		return;
	}

	value->x = getValue<int>( x );
	value->y = getValue<int>( y );
	value->z = getValue<int>( z );
}

void getValue( Dart_Handle handle, ci::vec3 *value )
{
	Dart_Handle x = getField( handle, "x" );
	Dart_Handle y = getField( handle, "y" );
	Dart_Handle z = getField( handle, "z" );

	if( Dart_IsError( x ) || Dart_IsError( y ) ) {
		CI_LOG_E( "expected handle to have fields 'x', 'y', and 'z'" );
		return;
	}

	value->x = getValue<float>( x );
	value->y = getValue<float>( y );
	value->z = getValue<float>( z );
}

void getValue( Dart_Handle handle, std::string *value )
{
	if( ! Dart_IsString( handle ) ) {
		CI_LOG_E( "expected handle to be of type string" );
		return;
	}

	*value = toString( handle );
}

bool hasFunction( Dart_Handle handle, const string &name )
{
	Dart_Handle result = Dart_LookupFunction( handle, toDart( name ) );
	CIDART_CHECK( result );
	return ! Dart_IsNull( result );
}

Dart_Handle callFunction( Dart_Handle target, const string &name, int numArgs, Dart_Handle *args )
{
	Dart_Handle result = Dart_Invoke( target, toDart( name ), numArgs, args );
	CIDART_CHECK( result );
	return result;
}

Dart_Handle getField( Dart_Handle container, const string &name )
{
	return Dart_GetField( container, toDart( name ) );
}

string getTypeName( Dart_Handle handle )
{
	Dart_Handle instanceType = Dart_InstanceGetType( handle );
	CIDART_CHECK( instanceType );
	Dart_Handle typeName = Dart_TypeName( instanceType );
	CIDART_CHECK( typeName );

	return toString( typeName );
}

// TODO: use Dart_isMap()
bool isMap( Dart_Handle handle )
{
	Dart_Handle coreLib = Dart_LookupLibrary( toDart( "dart:core" ) );
	CIDART_CHECK( coreLib );

	Dart_Handle type = Dart_GetType( coreLib, toDart( "Map" ), 0, nullptr );
	CIDART_CHECK( type );

	bool result;
	CIDART_CHECK( Dart_ObjectIsType( handle, type, &result ) );

	return result;
}

// ???: rename to isCinderType?
// - maybe shoud be using Dart_GetType as well
bool isCinderClass( Dart_Handle handle, const char *className )
{
	Dart_Handle cinderLib = Dart_LookupLibrary( toDart( "cinder" ) );
	CIDART_CHECK( cinderLib );
	Dart_Handle cinderClass = Dart_GetClass( cinderLib, toDart( className ) );
	CIDART_CHECK( cinderClass );

	bool result;
	CIDART_CHECK( Dart_ObjectIsType( handle, cinderClass, &result ) );
	return result;
}

string printNativeArgumentsToString( Dart_NativeArguments args, bool printMethodNames )
{
	stringstream stream;

	int argCount = Dart_GetNativeArgumentCount( args );
	stream << "arg count: " << argCount << endl;

	for( int i = 0; i < argCount; i++ ) {
		Dart_Handle handle = Dart_GetNativeArgument( args, i );

		Dart_Handle instanceType = Dart_InstanceGetType( handle );
		CIDART_CHECK( instanceType );
		string typeName = cidart::toString( Dart_TypeName( instanceType ) );

		stream << "\t[" << i << "]: type: " << typeName << endl;

		if( printMethodNames ) {
			Dart_Handle functionNamesList = Dart_GetFunctionNames( instanceType );
			CIDART_CHECK( functionNamesList );

			CI_ASSERT( Dart_IsList( functionNamesList ) );

			intptr_t length = 0;
			CIDART_CHECK( Dart_ListLength( functionNamesList, &length ) );

			stream << "\t\t funcNames: ";
			for( intptr_t i = 0; i < length; i++ ) {
				Dart_Handle elem = Dart_ListGetAt( functionNamesList, i );
				CIDART_CHECK( elem );

				string funcName = cidart::toString( elem );
				stream << funcName << ", ";
				
			}
			stream << endl;
		}
	}

	return stream.str();
}

} // namespace cidart
