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

Script::Options& Script::Options::native( const string &dartFuncName, Dart_NativeFunction nativeFn )
{
	mNativeCallbackMap[dartFuncName] = nativeFn;
	return *this;
}

Script::Options& Script::Options::native( const string &dartFuncName, const FunctionCallback &nativeFn )
{
	mFunctionCallbackMap[dartFuncName] = nativeFn;
	return *this;
}

Script::Script( const fs::path &sourcePath, const Options &options )
	: mIsolate( nullptr ), mMainScriptPath( sourcePath ),
		mNativeCallbackMap( options.getNativeCallbackMap() ), mFunctionCallbackMap( options.getFunctionCallbackMap() ), mReceiveMapCallback( options.getReceiveMapCallback() )
{
	init();
}

Script::Script( const DataSourceRef &source, const Options &options )
	: mIsolate( nullptr ), mMainScriptPath( source->getFilePath() ),
		mNativeCallbackMap( options.getNativeCallbackMap() ), mFunctionCallbackMap( options.getFunctionCallbackMap() ), mReceiveMapCallback( options.getReceiveMapCallback() )
{
	init();
}

void Script::init()
{
	if( mMainScriptPath.empty() || ! fs::exists( mMainScriptPath ) || fs::is_directory( mMainScriptPath ) )
		throw DartException( "invalid script path: " + mMainScriptPath.string() );

	mNativeCallbackMap["printNative"] = printNative;
	mFunctionCallbackMap["toCinder"] = bind( &Script::toCinder, this, placeholders::_1 ); // TODO: store in mNativeCallbackMap
	mNativeCallbackMap["cidart::callNative0"] = callNativeFunctionHandler;
	mNativeCallbackMap["cidart::callNative1"] = callNativeFunctionHandler;
	mNativeCallbackMap["cidart::callNative2"] = callNativeFunctionHandler;
	mNativeCallbackMap["cidart::callNative3"] = callNativeFunctionHandler;
	mNativeCallbackMap["cidart::callNative4"] = callNativeFunctionHandler;
	mNativeCallbackMap["cidart::callNative5"] = callNativeFunctionHandler;

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

Dart_Handle Script::invoke( const string &functionName, int argc, Dart_Handle *args )
{
	Dart_Handle library = Dart_RootLibrary();
	CI_ASSERT( ! Dart_IsNull( library ) );

	Dart_Handle nameHandle = toDart( functionName.c_str() );
	Dart_Handle result = Dart_Invoke( library, nameHandle, argc, args );

	if( Dart_IsError( result ) )
		throw DartException( Dart_GetError( result ) );

	return result;
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
	DartScope enterScope;

	string name = getValue<string>( nameHandle );

	Script *script = static_cast<Script *>( Dart_CurrentIsolateData() );
	auto& callbackMap = script->mNativeCallbackMap;
	auto functionIt = callbackMap.find( name );
	if( functionIt != callbackMap.end() )
		return functionIt->second;

	return nullptr;
}

// ----------------------------------------------------------------------------------------------------
// MARK: - Native Callbacks
// ----------------------------------------------------------------------------------------------------

// Static
void Script::callNativeFunctionHandler( Dart_NativeArguments args )
{
	Script *script = static_cast<Script *>( Dart_CurrentIsolateData() );

	const string &callbackKey = getArg<string>( args, 0 ); // first arg is always the callbackId
	auto &callbackMap = script->mFunctionCallbackMap;

	auto functionIt = callbackMap.find( callbackKey );
	if( functionIt != callbackMap.end() )
		functionIt->second( args );
	else
		throwException( "Unhandled native callback for function with key: " + callbackKey );
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
