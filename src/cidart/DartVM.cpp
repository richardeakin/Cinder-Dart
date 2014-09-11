// Copyright (c) 2013, Richard Eakin and the Dart project authors.
// Use of this source code (and the Dart VM) is governed by a
// BSD-style license that can be found in the LICENSE.txt file.

#include "cidart/DartVM.h"
#include "cidart/DartTypes.h"
#include "cidart/DartDebug.h"

#include "cinder/app/App.h"
#include "cinder/Utilities.h"
#include "cinder/CinderAssert.h"

#include <map>

#define PROFILE_LOAD_TIME 0

#if PROFILE_LOAD_TIME
	#include "cinder/Timer.h"
#endif

using namespace std;
using namespace ci;

namespace cidart {

DartVM::DartVM()
	: mIsolate( nullptr )
{
	// TODO: pass in vm flags at construction.
	mVMFlags.push_back( "--checked" );
	mVMFlags.push_back( "--no-profile" ); // currently dart's profiler seems to be blocking the main thread when debugging in xcode - this disables it for now
//	mVMFlags.push_back( "--print-flags" );

	const char **vmCFlags = (const char **)malloc( mVMFlags.size() * sizeof( const char * ) );
	for( size_t i = 0; i < mVMFlags.size(); i++ )
		vmCFlags[i] = mVMFlags[i].c_str();


	mNativeFunctionMap = {
		{ "printNative", printNative },
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
								closeFileCallback,
								NULL,
								NULL );
	CI_VERIFY( success );
}

void DartVM::loadScript( ci::DataSourceRef script )
{
#if PROFILE_LOAD_TIME
	Timer timer( true );
#endif

	mMainScriptPath = script->getFilePath();

	Dart_Isolate currentIsolate = Dart_CurrentIsolate();
	if( currentIsolate && currentIsolate == mIsolate ) {
		CI_LOG_V( "isolate already loaded, shutting down first" );;
		Dart_ShutdownIsolate();
	}

	if( ! mSnapshot )
		mSnapshot = app::loadResource( "snapshot_gen.bin" );

	const char *scriptPath = script->getFilePath().c_str();
	char *error;
	mIsolate = createIsolateCallback( scriptPath, "main", this, &error );
	if( ! mIsolate ) {
		CI_LOG_E( "could not create isolate: " << error );;
		CI_ASSERT( false );
	}

	DartScope enterScope;

	loadCinderDartLib();

	Dart_Handle url = toDart( scriptPath );
	string scriptContents = loadString( script );

//	CI_LOG_V( "script contents: " << scriptContents );;

	Dart_Handle source = toDart( scriptContents );
	CIDART_CHECK( source );
	CIDART_CHECK_RETURN( Dart_LoadScript( url, source, 0, 0 ) );
	CIDART_CHECK( Dart_SetNativeResolver( Dart_RootLibrary(), resolveNameHandler, NULL ) );

	// finalize script and invoke main.
	CIDART_CHECK( Dart_FinalizeLoading( false ) );
	invoke( "main" );

#if PROFILE_LOAD_TIME
	CI_LOG_V( "load complete. elapsed time: " << timer.getSeconds() << " seconds." );
#endif
}

void DartVM::loadCinderDartLib()
{
	string script = getCinderDartScript();
	Dart_Handle source = toDart( script );
	CIDART_CHECK( source );

	Dart_Handle cinderDartLib = Dart_LoadLibrary( toDart( "cinder" ), source, 0, 0 );
	CIDART_CHECK( cinderDartLib );
	CIDART_CHECK( Dart_SetNativeResolver( cinderDartLib, resolveNameHandler, NULL ) );

	// finalize any scripts loaded, needs to be done before the libs can be looked up and modified below
	CIDART_CHECK( Dart_FinalizeLoading( false ) );

	// swap in custom _printClosure to enable print() in dart
	Dart_Handle internalLib = Dart_LookupLibrary( toDart( "dart:_internal" ) );
	CIDART_CHECK( internalLib );
	Dart_Handle print = Dart_GetField( cinderDartLib, toDart( "_printClosure" ) );
	CIDART_CHECK( print );
	CIDART_CHECK( Dart_SetField( internalLib, toDart( "_printClosure" ), print ) );
}

void DartVM::invoke( const string &functionName, int argc, Dart_Handle* args )
{
	Dart_Handle library = Dart_RootLibrary();
	CI_ASSERT( ! Dart_IsNull( library ) );

	Dart_Handle nameHandle = toDart( functionName.c_str() );
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

void DartVM::setCinderDartScriptPath( const fs::path &scriptPath )
{
	CI_ASSERT( fs::exists( scriptPath ) );

	mCinderDartScriptPath = scriptPath;
}

void DartVM::setCinderDartScriptResource( const ci::DataSourceRef &scriptResource )
{
	mCinderDartScriptResource = scriptResource;
}

string DartVM::getCinderDartScript()
{
	if( mCinderDartScriptResource )
		return loadString( mCinderDartScriptResource );

#if defined( CINDER_COCOA )
	if( mCinderDartScriptPath.empty() ) {
		// attempt to load it as a resource added to the project
		auto resource = app::loadResource( "cinder.dart" );
		mCinderDartScriptPath = resource->getFilePath();
	}
#endif

	return loadString( loadFile( mCinderDartScriptPath ) );
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

	CI_LOG_V( "Creating isolate " << script_uri << ", " << main );
	Dart_Isolate isolate = Dart_CreateIsolate( script_uri, main, snapshotData, data, error );
	if ( ! isolate ) {
		CI_LOG_E( "Couldn't create isolate: " << *error );
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
	CI_LOG_V( "continuing.." );
	return true;
}

// static
void DartVM::unhandledExceptionCallback( Dart_Handle error )
{
	 CI_LOG_E( Dart_GetError( error ) );
}

// static
void DartVM::shutdownIsolateCallback( void *callbackData )
{
	CI_LOG_V( "bang" );
}

// file callbacks have been copied verbatum from included sample... plus verbose logging. don't event know yet if we need them
// static
void* DartVM::openFileCallback(const char* name, bool write)
{
	CI_LOG_V( "name: " << name << ", write mode: " << boolalpha << write << dec );
	
	return fopen(name, write ? "w" : "r");
}

// static
void DartVM::readFileCallback(const uint8_t** data, intptr_t* fileLength, void* stream )
{
	CI_LOG_V( "bang" );
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
	CI_LOG_V( "bang" );

	fwrite(data, 1, length, reinterpret_cast<FILE*>(file));
}

// static
void DartVM::closeFileCallback(void* file)
{
	CI_LOG_V( "bang" );

	fclose(reinterpret_cast<FILE*>(file));
}

namespace {

const std::string SCHEME_STRING_PACKAGE = "package:";

fs::path resolvePackageImportPath( const string &packageUrlString, const fs::path &mainScriptPath )
{
	const size_t lenPackageString = SCHEME_STRING_PACKAGE.length();
	string subPath = packageUrlString.substr( lenPackageString, packageUrlString.size() - lenPackageString );

	return mainScriptPath.parent_path() / "packages" / subPath;
}

// TESTING ONLY - currently a no method found exception is thrown about dart resolve() method
Dart_Handle getFilePathFromUri( Dart_Handle script_uri, Dart_Handle builtin_lib )
{
	const int numArgs = 1;
	Dart_Handle dartArgs[numArgs];
	dartArgs[0] = script_uri;

	return Dart_Invoke( builtin_lib, toDart( "_filePathFromUri" ), numArgs, dartArgs );
}

} // anonymous namespace

// static
Dart_Handle DartVM::libraryTagHandler( Dart_LibraryTag tag, Dart_Handle library, Dart_Handle urlHandle )
{
	if( tag == Dart_kCanonicalizeUrl )
		return urlHandle;

	string urlString = getValue<string>( urlHandle );

	DartVM *dartVM = static_cast<DartVM *>( Dart_CurrentIsolateData() );

	if( tag == Dart_kImportTag ) {

		// try to load a pub-style package
		auto pos = urlString.find( SCHEME_STRING_PACKAGE );
		if( pos == 0 ) {
			// search for package relative to main script root, in package folder

			auto resolvedPath = resolvePackageImportPath( urlString, dartVM->mMainScriptPath );
			dartVM->mImportedLibraries[urlString] = resolvedPath;

			string libString = loadString( loadFile( resolvedPath ) );
			Dart_Handle source = toDart( libString );
			CIDART_CHECK( source );

			Dart_Handle loadedLib = Dart_LoadLibrary( urlHandle, source, 0, 0  );
			CIDART_CHECK( loadedLib );

			return loadedLib;
		}

		// try to load file relative to main script path
		auto fullPath = dartVM->mMainScriptPath.parent_path() / urlString;
		if( fs::exists( fullPath ) ) {

			string fileString = loadString( loadFile( fullPath ) );

			Dart_Handle libString = toDart( fileString );
			Dart_Handle loadedHandle = Dart_LoadLibrary( urlHandle, libString, 0, 0 );
			CIDART_CHECK( loadedHandle );

			CIDART_CHECK( Dart_SetNativeResolver( loadedHandle, resolveNameHandler, NULL ) );

			return loadedHandle;
		}
	}
	else if( tag == Dart_kSourceTag ) {
		DartVM *dartVM = static_cast<DartVM *>( Dart_CurrentIsolateData() );

		Dart_Handle libraryUrl = Dart_LibraryUrl( library );
		string libraryUrlString = getValue<string>( libraryUrl );
		auto pathIt = dartVM->mImportedLibraries.find( libraryUrlString );

		CI_ASSERT( pathIt != dartVM->mImportedLibraries.end() );

		const auto &libFolder = pathIt->second.parent_path();
		auto resolvedPath = libFolder / urlString;

		string sourceString = loadString( loadFile( resolvedPath ) );
		Dart_Handle source = toDart( sourceString );

		Dart_Handle loadedHandle = Dart_LoadSource( library, urlHandle, source, 0, 0 );
		CIDART_CHECK( loadedHandle );
		return loadedHandle;
	}

	CI_ASSERT_NOT_REACHABLE();
	return nullptr;
}

// static
Dart_NativeFunction DartVM::resolveNameHandler( Dart_Handle nameHandle, int numArgs, bool* auto_setup_scope )
{
	CI_ASSERT( Dart_IsString( nameHandle ) );

	DartScope enterScope;

	string name = getValue<string>( nameHandle );

	DartVM *dartVm = static_cast<DartVM *>( Dart_CurrentIsolateData() );
	auto& functionMap = dartVm->mNativeFunctionMap;
	auto functionIt = functionMap.find( name );
	if( functionIt != functionMap.end() )
		return functionIt->second;

	return nullptr;
}

// static
void DartVM::printNative( Dart_NativeArguments arguments )
{
	DartScope enterScope;
	Dart_Handle handle = Dart_GetNativeArgument( arguments, 0 );
	CIDART_CHECK( handle );

	ci::app::console() << "|dart| " << getValue<string>( handle ) << std::endl;
}

// TODO: see if I can use Dart_ObjectIsType to ensure the class is of type Map
// - actually it looks like there is new API for Map's - check those out too
// static
void DartVM::toCinder( Dart_NativeArguments arguments )
{
	DartScope enterScope;

	DartVM *cd = static_cast<DartVM *>( Dart_CurrentIsolateData() );
	if( ! cd->mReceiveMapCallback ) {
		CI_LOG_E( "no ReceiveMapCallback, returning." );
		return;
	}

	Dart_Handle handle = Dart_GetNativeArgument( arguments, 0 );

	if( ! Dart_IsInstance( handle ) ) {
		CI_LOG_E( "not a dart instance." );
		return;
	}

	string typeName = getTypeName( handle );
	CI_LOG_V( "type name: " << typeName );

	if( ! isMap( handle ) ) {
		CI_LOG_E( "expected object of type map" );
		return;
	}

	// get keys:

	Dart_Handle length = getField( handle, "length" );

	int numEntries = getValue<int>( length );
	Dart_Handle keys = getField( handle, "keys" );

	Dart_Handle lengthIter = getField( keys, "length" );
	int lenIter = getValue<int>( lengthIter );
	CI_ASSERT( numEntries == lenIter );

	DataMap map;

	for( size_t i = 0; i < lenIter; i++ ) {
		Dart_Handle args[] = { Dart_NewInteger( i ) };
		Dart_Handle key = callFunction( keys, "elementAt", 1, args );
		string keyString = getValue<string>( key );

		args[0] = key;
		Dart_Handle value = callFunction( handle, "[]", 1, args );

		// hand user the Dart_Handle
		map[keyString] = value;
	}

	cd->mReceiveMapCallback( map );
}

} // namespace cidart