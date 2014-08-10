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
	// TODO: pass in vm flags at construction.

	mVMFlags.push_back( "--enable-checked-mode" );
	mVMFlags.push_back( "--no-profile" ); // currently dart's profiler seems to be blocking the main thread when debugging in xcode - this disables it for now
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
								closeFileCallback,
								NULL,
								NULL );
	CI_VERIFY( success );
}

void DartVM::loadScript( ci::DataSourceRef script )
{
	mMainScriptPath = script->getFilePath();

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
	Dart_Handle cinderDartLib = Dart_LookupLibrary( Dart_NewStringFromCString( "cinder" ) );
	CIDART_CHECK( cinderDartLib );

	Dart_Handle internalLib = Dart_LookupLibrary( Dart_NewStringFromCString( "dart:_internal" ) );
	CIDART_CHECK( internalLib );
	Dart_Handle print = Dart_GetField( cinderDartLib, Dart_NewStringFromCString( "_printClosure" ) );
	CIDART_CHECK( print );
	CIDART_CHECK( Dart_SetField( internalLib, Dart_NewStringFromCString( "_printClosure" ), print ) );

	Dart_Handle rootLib = Dart_RootLibrary();
	CI_ASSERT( ! Dart_IsNull( rootLib ) );

	CIDART_CHECK( Dart_SetNativeResolver( rootLib, resolveNameHandler, NULL ) );

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

	LOG_V( "Creating isolate " << script_uri << ", " << main );
	Dart_Isolate isolate = Dart_CreateIsolate( script_uri, main, snapshotData, data, error );
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

	return Dart_Invoke( builtin_lib, newString( "_filePathFromUri" ), numArgs, dartArgs );
}

} // anonymous namespace

// static
Dart_Handle DartVM::libraryTagHandler( Dart_LibraryTag tag, Dart_Handle library, Dart_Handle urlHandle )
{
	if( tag == Dart_kCanonicalizeUrl )
		return urlHandle;

	string urlString = getString( urlHandle );

	DartVM *dartVM = static_cast<DartVM *>( Dart_CurrentIsolateData() );

	if( tag == Dart_kImportTag ) {
		if( urlString == "cinder" ) {
			// load the cinder lib from special location

			DartVM *dartVm = static_cast<DartVM *>( Dart_CurrentIsolateData() );

			string script = dartVm->getCinderDartScript();
			Dart_Handle source = Dart_NewStringFromCString( script.c_str() );
			CIDART_CHECK( source );

			Dart_Handle cinderDartLib = Dart_LoadLibrary( urlHandle, source );
			CIDART_CHECK( cinderDartLib );

			CIDART_CHECK( Dart_SetNativeResolver( cinderDartLib, resolveNameHandler, NULL ) );

			return cinderDartLib;
		}

		auto pos = urlString.find( SCHEME_STRING_PACKAGE );
		if( pos == 0 ) {
			// search for package relative to main script root, in package folder

			auto resolvedPath = resolvePackageImportPath( urlString, dartVM->mMainScriptPath );
			dartVM->mImportedLibraries[urlString] = resolvedPath;

			string libString = loadString( loadFile( resolvedPath ) );
			Dart_Handle source = Dart_NewStringFromCString( libString.c_str() );
			CIDART_CHECK( source );

			Dart_Handle loadedLib = Dart_LoadLibrary( urlHandle, source );
			CIDART_CHECK( loadedLib );

			return loadedLib;
		}
	}
	else if( tag == Dart_kSourceTag ) {
		DartVM *dartVM = static_cast<DartVM *>( Dart_CurrentIsolateData() );

		Dart_Handle libraryUrl = Dart_LibraryUrl( library );
		string libraryUrlString = getString( libraryUrl );
		auto pathIt = dartVM->mImportedLibraries.find( libraryUrlString );

		CI_ASSERT( pathIt != dartVM->mImportedLibraries.end() );

		const auto &libFolder = pathIt->second.parent_path();
		auto resolvedPath = libFolder / urlString;

		string sourceString = loadString( loadFile( resolvedPath ) );
		Dart_Handle source = newString( sourceString.c_str() );

		Dart_Handle loadedHandle = Dart_LoadSource( library, urlHandle, source );
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