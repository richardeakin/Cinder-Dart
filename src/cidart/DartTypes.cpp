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
		LOG_E( "expected handle to be either of type float or int" );
}
	
} // anonymous namespace

void console( Dart_NativeArguments arguments )
{
	DartScope enterScope;
	Dart_Handle handle = Dart_GetNativeArgument( arguments, 0 );
	CIDART_CHECK( handle );

	ci::app::console() << "|dart| " << getString( handle ) << std::endl;
}

Dart_Handle newString( const char* str )
{
	return Dart_NewStringFromCString( str );
}

Dart_Handle newInt( int i )
{
	return Dart_NewInteger( i );
}

string getString( Dart_Handle handle )
{
	const char *result;
	CIDART_CHECK( Dart_StringToCString( handle, &result ) );
	return string( result );
}

bool getBool( Dart_Handle handle )
{
	bool result;
	CIDART_CHECK( Dart_BooleanValue( handle, &result ) );
	return result;
}

int getInt( Dart_Handle handle )
{
	int result;
	getValue( handle, &result );
	return result;
}

float getFloat( Dart_Handle handle )
{
	float result;
	getValue( handle, &result );
	return result;
}

ci::ColorA getColor( Dart_Handle handle )
{
	ci::ColorA result;
	getValue( handle, &result );
	return result;
}

float getFloatForKey( Dart_Handle mapHandle, const char *key )
{
	CI_ASSERT( isMap( mapHandle ) );

	Dart_Handle args[] = { cidart::newString( key ) };
	Dart_Handle valueHandle = cidart::callFunction( mapHandle, "[]", 1, args );
	CIDART_CHECK( valueHandle );

	return cidart::getFloat( valueHandle );
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
		LOG_E( "expected handle to have fields 'r', 'g' and 'b'" );
		return;
	}

	value->r = getFloat( r );
	value->g = getFloat( g );
	value->b = getFloat( b );
}

void getValue( Dart_Handle handle, ci::ColorA *value )
{
	Dart_Handle r = getField( handle, "r" );
	Dart_Handle g = getField( handle, "g" );
	Dart_Handle b = getField( handle, "b" );
	Dart_Handle a = getField( handle, "a" );

	if( Dart_IsError( r ) || Dart_IsError( g ) || Dart_IsError( b ) || Dart_IsError( a ) ) {
		LOG_E( "expected handle to have fields 'r', 'g', 'b' and 'a'" );
		return;
	}

	value->r = getFloat( r );
	value->g = getFloat( g );
	value->b = getFloat( b );
	value->a = getFloat( a );
}

void getValue( Dart_Handle handle, ci::Vec2i *value )
{
	Dart_Handle x = getField( handle, "x" );
	Dart_Handle y = getField( handle, "y" );

	if( Dart_IsError( x ) || Dart_IsError( y ) ) {
		LOG_E( "expected handle to have fields 'x' and 'y'" );
		return;
	}

	value->x = getInt( x );
	value->y = getInt( y );
}

void getValue( Dart_Handle handle, ci::Vec2f *value )
{
	Dart_Handle x = getField( handle, "x" );
	Dart_Handle y = getField( handle, "y" );

	if( Dart_IsError( x ) || Dart_IsError( y ) ) {
		LOG_E( "expected handle to have fields 'x' and 'y'" );
		return;
	}

	value->x = getFloat( x );
	value->y = getFloat( y );
}

void getValue( Dart_Handle handle, ci::Vec3i *value )
{
	Dart_Handle x = getField( handle, "x" );
	Dart_Handle y = getField( handle, "y" );
	Dart_Handle z = getField( handle, "z" );

	if( Dart_IsError( x ) || Dart_IsError( y ) ) {
		LOG_E( "expected handle to have fields 'x', 'y', and 'z'" );
		return;
	}

	value->x = getInt( x );
	value->y = getInt( y );
	value->z = getInt( z );
}

void getValue( Dart_Handle handle, ci::Vec3f *value )
{
	Dart_Handle x = getField( handle, "x" );
	Dart_Handle y = getField( handle, "y" );
	Dart_Handle z = getField( handle, "z" );

	if( Dart_IsError( x ) || Dart_IsError( y ) ) {
		LOG_E( "expected handle to have fields 'x', 'y', and 'z'" );
		return;
	}

	value->x = getFloat( x );
	value->y = getFloat( y );
	value->z = getFloat( z );
}

void getValue( Dart_Handle handle, std::string *value )
{
	if( ! Dart_IsString( handle ) ) {
		LOG_E( "expected handle to be of type string" );
		return;
	}

	*value = getString( handle );
}

bool hasFunction( Dart_Handle handle, const string &name )
{
	Dart_Handle result = Dart_LookupFunction( handle, newString( name.c_str() ) );
	CIDART_CHECK( result );
	return ! Dart_IsNull( result );
}

Dart_Handle callFunction( Dart_Handle target, const string &name, int numArgs, Dart_Handle *args )
{
	Dart_Handle result = Dart_Invoke( target, newString( name.c_str() ), numArgs, args );
	CIDART_CHECK( result );
	return result;
}

Dart_Handle getField( Dart_Handle container, const string &name )
{
	return Dart_GetField( container, newString( name.c_str() ) );
}

string getTypeName( Dart_Handle handle )
{
	Dart_Handle instanceType = Dart_InstanceGetType( handle );
	CIDART_CHECK( instanceType );
	Dart_Handle typeName = Dart_TypeName( instanceType );
	CIDART_CHECK( typeName );

	return getString( typeName );
}

// TODO: use Dart_isMap()
bool isMap( Dart_Handle handle )
{
	Dart_Handle coreLib = Dart_LookupLibrary( newString( "dart:core" ) );
	CIDART_CHECK( coreLib );

	Dart_Handle type = Dart_GetType( coreLib, newString( "Map" ), 0, nullptr );
	CIDART_CHECK( type );

	bool result;
	CIDART_CHECK( Dart_ObjectIsType( handle, type, &result ) );

	return result;
}

// ???: rename to isCinderType?
// - maybe shoud be using Dart_GetType as well
bool isCinderClass( Dart_Handle handle, const char *className )
{
	Dart_Handle cinderLib = Dart_LookupLibrary( newString( "cinder" ) );
	CIDART_CHECK( cinderLib );
	Dart_Handle cinderClass = Dart_GetClass( cinderLib, newString( className ) );
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
		string typeName = cidart::getString( Dart_TypeName( instanceType ) );

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

				string funcName = cidart::getString( elem );
				stream << funcName << ", ";
				
			}
			stream << endl;
		}
	}

	return stream.str();
}

} // namespace cidart
