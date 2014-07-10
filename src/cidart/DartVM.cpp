// Copyright (c) 2013, Richard Eakin and the Dart project authors.
// Use of this source code (and the Dart itself) is governed by a
// BSD-style license that can be found in the LICENSE.txt file.

#include "cidart/DartVM.h"
#include "cidart/DartTypes.h"
#include "cidart/DartDebug.h"

#include "cinder/app/App.h"
#include "cinder/Utilities.h"
#include "cinder/CinderAssert.h"

#include <map>

using namespace std;
using namespace ci;

namespace cidart {

DartVM::DartVM()
	: mIsolate( nullptr )
{
	mVMFlags.push_back( "--enable-checked-mode" );
//	mVMFlags.push_back( "--print-flags" );

	const char **vmCFlags = (const char **)malloc( mVMFlags.size() * sizeof( const char * ) );
	for( size_t i = 0; i < mVMFlags.size(); i++ )
		vmCFlags[i] = mVMFlags[i].c_str();


	mNativeFunctionMap = {
		{ "console", console },
		{ "toCinder", toCinder }
	};


	// setting VM startup options
	bool success = Dart_SetVMFlags( mVMFlags.size(), vmCFlags );
	CI_VERIFY( success );
	free( vmCFlags );

	success = Dart_Initialize(	createIsolateCallback,
								interruptIsolateCallback,
								unhandledExceptionCallback,
								shutdownIsolateCallback,
								openFileCallback,
								readFileCallback,
								writeFileCallback,
								closeFileCallback );
	CI_VERIFY( success );
}

void DartVM::loadScript( ci::DataSourceRef script )
{
	Dart_Isolate currentIsolate = Dart_CurrentIsolate();
	if( currentIsolate && currentIsolate == mIsolate ) {
		LOG_V( "isolate already loaded, shutting down first" );;
		Dart_ShutdownIsolate();
	}

	if( ! mSnapshot )
		mSnapshot = app::loadResource( "snapshot_gen.bin" );

	const char *scriptPath = script->getFilePath().c_str();
	char *error;
	mIsolate = createIsolateCallback( scriptPath, "main", this, &error );
	if( ! mIsolate ) {
		LOG_E( "could not create isolate: " << error );;
		CI_ASSERT( false );
	}

	DartScope enterScope;

	Dart_Handle url = newString( scriptPath );
	string scriptContents = loadString( script );

//	LOG_V( "script contents: " << scriptContents );;

	Dart_Handle source = Dart_NewStringFromCString( scriptContents.c_str() );
	CIDART_CHECK( source );
	CIDART_CHECK_RETURN( Dart_LoadScript( url, source, 0, 0 ) );

	// swap in custom _printClosure, which maps back to Log.
	Dart_Handle cinderDartLib = Dart_LookupLibrary( Dart_NewStringFromCString( "cinder" ) ); // TODO: import this first with Dart_LoadLibrary (right now it is imported from main.dart
	CIDART_CHECK( cinderDartLib );

	Dart_Handle internalLib = Dart_LookupLibrary( Dart_NewStringFromCString( "dart:_collection-dev" ) );
	CIDART_CHECK( internalLib );
	Dart_Handle print = Dart_GetField( cinderDartLib, Dart_NewStringFromCString( "_printClosure" ) );
	CIDART_CHECK( print );
	CIDART_CHECK( Dart_SetField( internalLib, Dart_NewStringFromCString( "_printClosure" ), print ) );

	Dart_Handle rootLib = Dart_RootLibrary();
	CI_ASSERT( ! Dart_IsNull( rootLib ) );

	CIDART_CHECK( Dart_SetNativeResolver( rootLib, resolveNameHandler ) );

	// I guess main needs to be manually invoked...
	// TODO: check dartium to see how it handles this part.
	//	- maybe it is handled with Dart_RunLoop() ?

	invoke( "main" );
}

void DartVM::invoke( const string &functionName, int argc, Dart_Handle* args )
{
	Dart_Handle library = Dart_RootLibrary();
	CI_ASSERT( ! Dart_IsNull( library ) );

	Dart_Handle nameHandle = Dart_NewStringFromCString( functionName.c_str() );
	Dart_Handle result = Dart_Invoke( library, nameHandle, argc, args );
	CIDART_CHECK_RETURN( result );

	// TODO: there was originally a note saying this probably isn't necessary.. try removing
	// Keep handling messages until the last active receive port is closed.
	result = Dart_RunLoop();
	CIDART_CHECK( result );

	return;
}

// static
string DartVM::getVersionString()
{
	return Dart_VersionString();
}

// TODO: make this load a string that user settable, so it is possible to use a modified cinder.dart
//	- however, need to make sure that a new DataSourceRef is still created, so that the content is hot-loadable
string DartVM::getCinderDartScript()
{
	DataSourceRef script = app::loadResource( "cinder.dart" );
	return loadString( script );
}

// ----------------------------------------------------------------------------------------------------
// MARK: - Dart Callbacks
// ----------------------------------------------------------------------------------------------------

// static
Dart_Isolate DartVM::createIsolateCallback( const char* script_uri, const char* main, void* data, char** error )
{
	DartVM *dartVm = reinterpret_cast<DartVM *>( data );

	CI_ASSERT_MSG( dartVm->mSnapshot, "cannot run without the core snapshot" );
	
	uint8_t *snapshotData = (uint8_t *)dartVm->mSnapshot->getBuffer().getData();

	LOG_V( "Creating isolate " << script_uri << ", " << main );
	Dart_Isolate isolate = Dart_CreateIsolate( script_uri, main, snapshotData, data, error );
	//	Dart_Isolate isolate = Dart_CreateIsolate( script_uri, main, NULL, data, error );
	if ( ! isolate ) {
		LOG_E( "Couldn't create isolate: " << *error );
		return nullptr;
	}

	// Set up the library tag handler for this isolate.
	DartScope enterScope;
	Dart_Handle result = Dart_SetLibraryTagHandler( libraryTagHandler );
	CIDART_CHECK( result );

	return isolate;
}

// static
bool DartVM::interruptIsolateCallback()
{
	LOG_V( "continuing.." );
	return true;
}

// static
void DartVM::unhandledExceptionCallback( Dart_Handle error )
{
	 LOG_E( Dart_GetError( error ) );
}

// static
void DartVM::shutdownIsolateCallback( void *callbackData )
{
	LOG_V( "bang" );
}

// file callbacks have been copied verbatum from included sample... plus verbose logging. don't event know yet if we need them
// static
void* DartVM::openFileCallback(const char* name, bool write)
{
	LOG_V( "name: " << name << ", write mode: " << boolalpha << write << dec );
	
	return fopen(name, write ? "w" : "r");
}

// static
void DartVM::readFileCallback(const uint8_t** data, intptr_t* fileLength, void* stream )
{
	LOG_V( "bang" );
	if (!stream) {
		*data = 0;
		*fileLength = 0;
	} else {
		FILE* file = reinterpret_cast<FILE*>(stream);

		// Get the file size.
		fseek(file, 0, SEEK_END);
		*fileLength = ftell(file);
		rewind(file);

		// Allocate data buffer.
		*data = new uint8_t[*fileLength];
		*fileLength = fread(const_cast<uint8_t*>(*data), 1, *fileLength, file);
	}
}

// static
void DartVM::writeFileCallback(const void* data, intptr_t length, void* file)
{
	LOG_V( "bang" );

	fwrite(data, 1, length, reinterpret_cast<FILE*>(file));
}

// static
void DartVM::closeFileCallback(void* file)
{
	LOG_V( "bang" );

	fclose(reinterpret_cast<FILE*>(file));
}

// loads the cinder lib if it is needed.
// static
Dart_Handle DartVM::libraryTagHandler( Dart_LibraryTag tag, Dart_Handle library, Dart_Handle urlHandle )
{
	if( tag == Dart_kCanonicalizeUrl )
		return urlHandle;

	string url = getString( urlHandle );
	if( url == "cinder" ) {
		DartVM *dartVm = static_cast<DartVM *>( Dart_CurrentIsolateData() );

		string script = dartVm->getCinderDartScript();
		Dart_Handle source = Dart_NewStringFromCString( script.c_str() );
		CIDART_CHECK( source );

		Dart_Handle library = Dart_LoadLibrary( urlHandle, source );
		CIDART_CHECK( library );

		CIDART_CHECK( Dart_SetNativeResolver( library, resolveNameHandler ) );

		return library;
	}

	CI_ASSERT( false && "unreachable" );
	return nullptr;
}

// static
Dart_NativeFunction DartVM::resolveNameHandler( Dart_Handle nameHandle, int numArgs )
{
	CI_ASSERT( Dart_IsString( nameHandle ) );

	DartScope enterScope;

	string name = getString( nameHandle );

	DartVM *dartVm = static_cast<DartVM *>( Dart_CurrentIsolateData() );
	auto& functionMap = dartVm->mNativeFunctionMap;
	auto functionIt = functionMap.find( name );
	if( functionIt != functionMap.end() )
		return functionIt->second;

	return nullptr;
}

// TODO: see if I can use Dart_ObjectIsType to ensure the class is of type Map
// - actually it looks like there is new API for Map's - check those out too
// static
void DartVM::toCinder( Dart_NativeArguments arguments )
{

	DartScope enterScope;

	DartVM *cd = static_cast<DartVM *>( Dart_CurrentIsolateData() );
	if( ! cd->mReceiveMapCallback ) {
		LOG_E( "no ReceiveMapCallback, returning." );
		return;
	}

	Dart_Handle handle = Dart_GetNativeArgument( arguments, 0 );

	if( ! Dart_IsInstance( handle ) ) {
		LOG_E( "not a dart instance." );
		return;
	}

	string typeName = getTypeName( handle );
	LOG_V( "type name: " << typeName );

	if( ! isMap( handle ) ) {
		LOG_E( "expected object of type map" );
		return;
	}

	// get keys:

	Dart_Handle length = getField( handle, "length" );

	int numEntries = getInt( length );
	Dart_Handle keys = getField( handle, "keys" );

	Dart_Handle lengthIter = getField( keys, "length" );
	int lenIter = getInt( lengthIter );
	CI_ASSERT( numEntries == lenIter );

	DataMap map;

	for( size_t i = 0; i < lenIter; i++ ) {
		Dart_Handle args[] = { newInt( i ) };
		Dart_Handle key = callFunction( keys, "elementAt", 1, args );
		string keyString = getString( key );

		args[0] = key;
		Dart_Handle value = callFunction( handle, "[]", 1, args );

		// hand user the Dart_Handle
		map[keyString] = value;
	}

	cd->mReceiveMapCallback( map );
}

} // namespace cidart