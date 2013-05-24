#include "DartTypes.h"
#include "debug.h"

using namespace std;

namespace cinderdart {

	void console( Dart_NativeArguments arguments ) {
		DartScope enterScope;
		Dart_Handle handle = Dart_GetNativeArgument( arguments, 0 );
		CHECK_DART( handle );

		LOG_V << getString( handle ) << std::endl;
	}

	Dart_Handle newString(const char* str) {
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

} // namespace cinderdart