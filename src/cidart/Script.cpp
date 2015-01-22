// Copyright (c) 2014, Richard Eakin
// Use of this source code (and the Dart VM) is governed by a
// BSD-style license that can be found in the LICENSE.txt file.

#include "cidart/Script.h"
#include "cidart/VM.h"
#include "cidart/Types.h"
#include "cidart/Debug.h"

#include "cinder/Utilities.h"

#define PROFILE_LOAD_TIME 0

using namespace ci;
using namespace std;

namespace cidart {

Script::Script( const fs::path &sourcePath, const Options &options )
	: mIsolate( nullptr ), mMainScriptPath( sourcePath ),
		mNativeCallbackMap( options.getNativeCallbackMap() ), mReceiveMapCallback( options.getReceiveMapCallback() )
{
	init();
}

Script::Script( const DataSourceRef &source, const Options &options )
	: mIsolate( nullptr ), mMainScriptPath( source->getFilePath() ),
		mNativeCallbackMap( options.getNativeCallbackMap() ), mReceiveMapCallback( options.getReceiveMapCallback() )
{
	init();
}

void Script::init()
{
	if( mMainScriptPath.empty() || ! fs::exists( mMainScriptPath ) || fs::is_directory( mMainScriptPath ) )
		throw DartException( "invalid script path: " + mMainScriptPath.string() );

	mNativeCallbackMap["printNative"] = printNative;
	mNativeCallbackMap["toCinder"] = bind( &Script::toCinder, this, placeholders::_1 );

	const char *sourcePath = mMainScriptPath.string().c_str();

	char *error;
	mIsolate = Script::createIsolateCallback( sourcePath, "main", NULL, this, &error );
	if( ! mIsolate )
		throw DartException( "could not create isolate, error: " + string( error ) );

	DartScope enterScope;

	VM::instance()->loadCinderDartLib();

	Dart_Handle url = toDart( sourcePath );
	string sourceStr = loadSourceImpl( mMainScriptPath );

	Dart_Handle sourceHandle = toDart( sourceStr );
	CIDART_CHECK( sourceHandle );

	Dart_Handle scriptHandle = Dart_LoadScript( url, sourceHandle, 0, 0 );
	if( Dart_IsError( scriptHandle ) )
		throw DartException( Dart_GetError( scriptHandle ) );

	CIDART_CHECK( Dart_SetNativeResolver( Dart_RootLibrary(), resolveNameHandler, NULL ) );

	// finalize script and invoke main.
	CIDART_CHECK( Dart_FinalizeLoading( false ) );
	invoke( "main" );
}

void Script::invoke( const string &functionName, int argc, Dart_Handle *args )
{
	Dart_Handle library = Dart_RootLibrary();
	CI_ASSERT( ! Dart_IsNull( library ) );

	Dart_Handle nameHandle = toDart( functionName.c_str() );
	Dart_Handle result = Dart_Invoke( library, nameHandle, argc, args );

	if( Dart_IsError( result ) )
		throw DartException( Dart_GetError( result ) );


	// TODO: there was originally a note saying this probably isn't necessary.. try removing
	// Keep handling messages until the last active receive port is closed.
	result = Dart_RunLoop();
	CIDART_CHECK( result );

	return;
}

string Script::loadSourceImpl( const fs::path &sourcePath )
{
	try {
		return loadString( loadFile( sourcePath ) );
	}
	catch( ci::StreamExc &exc ) {
		throw DartException( "failed to load source path (StreamExc caught): " + sourcePath.string() );
	}
}

// ----------------------------------------------------------------------------------------------------
// MARK: - Dart API Callbacks
// ----------------------------------------------------------------------------------------------------

// static
Dart_Isolate Script::createIsolateCallback( const char *scriptUri, const char *main, const char *packageRoot, void *callbackData, char **error )
{
	VM *vm = VM::instance();

	uint8_t *snapshotData = nullptr;
	auto snapshot = vm->getSnapShot();
	if( snapshot )
		snapshotData = (uint8_t *) snapshot->getBuffer().getData();

	Dart_Isolate isolate = Dart_CreateIsolate( scriptUri, main, snapshotData, callbackData, error );
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
Dart_Handle Script::libraryTagHandler( Dart_LibraryTag tag, Dart_Handle library, Dart_Handle urlHandle )
{
	if( tag == Dart_kCanonicalizeUrl )
		return urlHandle;

	string urlString = getValue<string>( urlHandle );

	Script *script = static_cast<Script *>( Dart_CurrentIsolateData() );

	if( tag == Dart_kImportTag ) {

		// try to load a pub-style package
		auto pos = urlString.find( SCHEME_STRING_PACKAGE );
		if( pos == 0 ) {
			// search for package relative to main script root, in package folder

			auto resolvedPath = resolvePackageImportPath( urlString, script->mMainScriptPath );
			script->mImportedLibraries[urlString] = resolvedPath;

			string libString = script->loadSourceImpl( resolvedPath );
			Dart_Handle source = toDart( libString );
			CIDART_CHECK( source );

			Dart_Handle loadedLib = Dart_LoadLibrary( urlHandle, source, 0, 0  );
			CIDART_CHECK( loadedLib );

			return loadedLib;
		}

		// try to load file relative to main script path
		auto fullPath = script->mMainScriptPath.parent_path() / urlString;
		if( fs::exists( fullPath ) ) {
			string fileString = script->loadSourceImpl( fullPath );

			Dart_Handle libString = toDart( fileString );
			Dart_Handle loadedHandle = Dart_LoadLibrary( urlHandle, libString, 0, 0 );
			if( Dart_IsError( loadedHandle ) )
				throw DartException( Dart_GetError( loadedHandle ) );

			CIDART_CHECK( Dart_SetNativeResolver( loadedHandle, resolveNameHandler, NULL ) );

			return loadedHandle;
		}
	}
	else if( tag == Dart_kSourceTag ) {
		Script *script = static_cast<Script *>( Dart_CurrentIsolateData() );

		Dart_Handle libraryUrl = Dart_LibraryUrl( library );
		string libraryUrlString = getValue<string>( libraryUrl );
		auto pathIt = script->mImportedLibraries.find( libraryUrlString );

		CI_ASSERT( pathIt != script->mImportedLibraries.end() );

		const auto &libFolder = pathIt->second.parent_path();
		auto resolvedPath = libFolder / urlString;

		string sourceString = script->loadSourceImpl( resolvedPath );
		Dart_Handle source = toDart( sourceString );

		Dart_Handle loadedHandle = Dart_LoadSource( library, urlHandle, source, 0, 0 );
		CIDART_CHECK( loadedHandle );
		return loadedHandle;
	}

	auto errorString = string( "could not resolve library with url: " ) + urlString;
	return Dart_NewApiError( errorString.c_str() );
}

// static
Dart_NativeFunction Script::resolveNameHandler( Dart_Handle nameHandle, int numArgs, bool *autoSetupScope )
{
	CI_ASSERT( Dart_IsString( nameHandle ) );

	DartScope enterScope;

	Script *script = static_cast<Script *>( Dart_CurrentIsolateData() );

	// store the name handle and return our callback handler
	script->mLatestNativeCallbackName = getValue<string>( nameHandle );
	return nativeCallbackHandler;
}

// ----------------------------------------------------------------------------------------------------
// MARK: - Native Callbacks
// ----------------------------------------------------------------------------------------------------

// Static
void Script::nativeCallbackHandler( Dart_NativeArguments args )
{
	Script *script = static_cast<Script *>( Dart_CurrentIsolateData() );

	const string &name = script->mLatestNativeCallbackName;
	auto& functionMap = script->mNativeCallbackMap;

	auto functionIt = functionMap.find( name );
	if( functionIt != functionMap.end() )
		functionIt->second( args );
	else {
		// TODO: it would be nice to throw, but I can't seem to be able to catch exceptions that come from native callbacks.
//		throw DartException( "Unhandled native callback for function name: " + name );
		CI_LOG_E( "Unhandled native callback for function name: " << name );
	}
}

// static
void Script::printNative( Dart_NativeArguments args )
{
	string message = getArg<string>( args, 0 );
	ci::app::console() << "|dart| " << message << std::endl;
}

void Script::toCinder( Dart_NativeArguments args )
{
	if( ! mReceiveMapCallback ) {
		CI_LOG_E( "no ReceiveMapCallback, returning." );
		return;
	}

	Dart_Handle mapHandle = Dart_GetNativeArgument( args, 0 );

	if( ! Dart_IsMap( mapHandle ) ) {
		CI_LOG_E( "expected object of type map" );
		return;
	}

	// get keys and create a map of keyString to Dart_Handle's
	Dart_Handle keysList = Dart_MapKeys( mapHandle );
	CIDART_CHECK( keysList );

	intptr_t numKeys;
	CIDART_CHECK( Dart_ListLength( keysList, &numKeys ) );

	InfoMap info;

	for( intptr_t i = 0; i < numKeys; i++ ) {
		Dart_Handle keyHandle = Dart_ListGetAt( keysList, i );
		Dart_Handle valueHandle = Dart_MapGetAt( mapHandle, keyHandle );

		string keyString = getValue<string>( keyHandle );
		info[keyString] = valueHandle;
	}
	
	mReceiveMapCallback( info );
}

} // namespace ciadart
