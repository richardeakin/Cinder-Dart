// Copyright (c) 2013, Richard Eakin and the Dart project authors.
// Use of this source code (and the Dart itself) is governed by a
// BSD-style license that can be found in the LICENSE.txt file.

#include "cidart/DartVM.h"
#include "cidart/debug.h"
#include "cidart/DartTypes.h"

#include "Resources.h"

#include "cinder/app/App.h"
#include "cinder/Utilities.h"

#include <map>

using namespace std;
using namespace ci;

namespace cidart {

// Dart_IsolateCreateCallback
Dart_Isolate createIsolateCallback( const char* script_uri, const char* main, void* data, char** error );
// Dart_IsolateInterruptCallback
bool interruptIsolateCallback();
// Dart_IsolateUnhandledExceptionCallback
void unhandledExceptionCallback( Dart_Handle error );
// Dart_IsolateShutdownCallback
void shutdownIsolateCallback( void *callbackData );
// Dart_FileOpenCallback
void* openFileCallback(const char* name, bool write);
// Dart_FileReadCallback
void readFileCallback(const uint8_t** data, intptr_t* fileLength, void* stream );
// Dart_FileWriteCallback
void writeFileCallback(const void* data, intptr_t length, void* file);
// Dart_FileCloseCallback
void closeFileCallback(void* file);
// Dart_LibraryTagHandler
Dart_Handle libraryTagHandler( Dart_LibraryTag tag, Dart_Handle library, Dart_Handle urlHandle );

// TODO: see if I can use Dart_ObjectIsType to ensure the class is of type Map
void toCinder( Dart_NativeArguments arguments ) {

	DartScope enterScope;

	DartVM *cd = static_cast<DartVM *>( Dart_CurrentIsolateData() );
	if( ! cd->mReceiveMapCallback ) {
		LOG_E << "no ReceiveMapCallback, returning." << endl;
		return;
	}

	Dart_Handle handle = Dart_GetNativeArgument( arguments, 0 );

	if( ! Dart_IsInstance( handle ) ) {
		LOG_E << "not a dart instance." << endl;
		return;
	}

	string className = getClassName( handle );
	LOG_V << "class name: " << className << endl;

	if( ! isMap( handle ) ) {
		LOG_E << "expected object of type map" << endl;
		return;
	}

	// get keys:

	Dart_Handle length = getField( handle, "length" );

	int numEntries = getInt( length );
	LOG_V << "numEntries: " << numEntries << endl;

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

Dart_NativeFunction resolveName( Dart_Handle handle, int argc )
{
	CI_ASSERT( Dart_IsString( handle ) );

	DartScope enterScope;

	const char* cname;
	CHECK_DART( Dart_StringToCString( handle, &cname ) );

	string name = getString( handle );

	DartVM *cd = static_cast<DartVM *>( Dart_CurrentIsolateData() );
	auto& functionMap = cd->mNativeFunctionMap;
	auto functionIt = functionMap.find( name );
	if( functionIt != functionMap.end() )
		return functionIt->second;

	return nullptr;
}


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


	LOG_V << "Setting VM Options" << endl;
	bool success = Dart_SetVMFlags( mVMFlags.size(), vmCFlags );
	CI_ASSERT( success );
	free( vmCFlags );

	success = Dart_Initialize( createIsolateCallback, interruptIsolateCallback, unhandledExceptionCallback, shutdownIsolateCallback, openFileCallback, readFileCallback, writeFileCallback, closeFileCallback );
	CI_ASSERT( success );

	LOG_V << "Dart_Initialize complete." << endl;
}

void DartVM::loadScript( ci::DataSourceRef script )
{
	Dart_Isolate currentIsolate = Dart_CurrentIsolate();
	if( currentIsolate && currentIsolate == mIsolate ) {
		LOG_V << "isolate already loaded, shutting down first" << endl;
		Dart_ShutdownIsolate();
	}

	const char *scriptPath = script->getFilePath().c_str();
	char *error;
	mIsolate = createIsolateCallback( scriptPath, "main", this, &error );
	if( ! mIsolate ) {
		LOG_E << "could not create isolate: " << error << endl;
		CI_ASSERT( false );
	}

	DartScope enterScope;

	Dart_Handle url = newString( scriptPath );
	string scriptContents = loadString( script );

//	LOG_V << "script contents: " << scriptContents << endl;

	Dart_Handle source = Dart_NewStringFromCString( scriptContents.c_str() );
	CHECK_DART( source );
	CHECK_DART_RETURN( Dart_LoadScript( url, source, 0, 0 ) );

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
	CHECK_DART_RETURN( result );

	// TODO: there was originally a note saying this probably isn't necessary.. try removing
	// Keep handling messages until the last active receive port is closed.
	result = Dart_RunLoop();
	CHECK_DART( result );

	return;
}

// ----------------------------------------------------------------------------------------------------
// MARK: - Dart Callbacks
// ----------------------------------------------------------------------------------------------------

// FIXME: loadResource may not always work at app creation time, and this may be called then
Dart_Isolate createIsolateCallback( const char* script_uri, const char* main, void* data, char** error )
{
	DataSourceRef snapshot = app::loadResource( RES_DART_SNAPSHOT );
	const uint8_t *snapshotData = (const uint8_t *)snapshot->getBuffer().getData();

	LOG_V << "Creating isolate " << script_uri << ", " << main << endl;
	Dart_Isolate isolate = Dart_CreateIsolate( script_uri, main, snapshotData, data, error );
	//	Dart_Isolate isolate = Dart_CreateIsolate( script_uri, main, NULL, data, error );
	if ( ! isolate ) {
		LOG_E << "Couldn't create isolate: " << *error << endl;
		return nullptr;
	}

	// Set up the library tag handler for this isolate.
	LOG_V << "Setting up library tag handler" << endl;
	DartScope enterScope;
	Dart_Handle result = Dart_SetLibraryTagHandler( libraryTagHandler );
	CHECK_DART( result );

	return isolate;
}

bool interruptIsolateCallback()
{
	LOG_V << "continuing.." << endl;
	return true;
}

void unhandledExceptionCallback( Dart_Handle error )
{
	LOG_E << Dart_GetError( error ) << endl;
}

void shutdownIsolateCallback( void *callbackData )
{
	LOG_V << "bang" << endl;
}

// file callbacks have been copied verbatum from included sample... plus verbose logging. don't event know yet if we need them
void* openFileCallback(const char* name, bool write)
{
	LOG_V << "name: " << name << ", write mode: " << boolalpha << write << dec << endl;
	
	return fopen(name, write ? "w" : "r");
}

void readFileCallback(const uint8_t** data, intptr_t* fileLength, void* stream )
{
	LOG_V << "bang" << endl;
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

void writeFileCallback(const void* data, intptr_t length, void* file)
{
	LOG_V << "bang" << endl;

	fwrite(data, 1, length, reinterpret_cast<FILE*>(file));
}

void closeFileCallback(void* file)
{
	LOG_V << "bang" << endl;

	fclose(reinterpret_cast<FILE*>(file));
}

// details of this method aren't really documented yet so I just log what I can here and move on.
Dart_Handle libraryTagHandler( Dart_LibraryTag tag, Dart_Handle library, Dart_Handle urlHandle )
{
	const char* url;
	Dart_StringToCString( urlHandle, &url );


	if( tag == kCanonicalizeUrl ) {
		LOG_V << "\tkCanonicalizeUrl" << endl;
		return urlHandle;
	}

	if( strcmp( url, "cinder" ) == 0 ) {
		DataSourceRef script = app::loadResource( RES_CINDER_DART );
		string scriptContents = loadString( script );

//		LOG_V << "script contents:\n\n" << scriptContents << endl;

		Dart_Handle source = Dart_NewStringFromCString( scriptContents.c_str() );
		CHECK_DART( source );

		Dart_Handle library = Dart_LoadLibrary( urlHandle, source );
		CHECK_DART( library );

		// swap in our custom _printClosure, which maps back to Log
		Dart_Handle corelib = Dart_LookupLibrary( Dart_NewStringFromCString( "dart:core" ) );
		CHECK_DART( corelib );
		Dart_Handle print = Dart_GetField( library, Dart_NewStringFromCString( "_printClosure" ) );
		CHECK_DART( print );
		CHECK_DART( Dart_SetField( corelib, Dart_NewStringFromCString( "_printClosure" ), print ) );


		CHECK_DART( Dart_SetNativeResolver( library, resolveName ) );

		return library;
	}


	CI_ASSERT( false );
	return nullptr;
}

} // namespace cidart