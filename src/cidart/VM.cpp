// Copyright (c) 2013, Richard Eakin and the Dart project authors.
// Use of this source code (and the Dart VM) is governed by a
// BSD-style license that can be found in the LICENSE.txt file.

#include "cidart/VM.h"
#include "cidart/Script.h"
#include "cidart/Types.h"
#include "cidart/Debug.h"
#include "cidart/Exception.h"

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

namespace {

template <typename T>
std::unique_ptr<T, decltype( &free )> mallocScoped( size_t count )
{
	return unique_ptr<T, decltype( &free )>( (T*)malloc( count * sizeof( T ) ), free );
}

} // anonymouse namespace

// static
VM* VM::instance()
{
	static std::unique_ptr<VM>	sInstance;
	if( ! sInstance )
		sInstance.reset( new VM );

	return sInstance.get();
}

VM::VM()
{
	// TODO: pass in vm flags at construction.
	mVMFlags.push_back( "--checked" );
	mVMFlags.push_back( "--no-profile" ); // currently dart's profiler seems to be blocking the main thread when debugging in xcode - this disables it for now
//	mVMFlags.push_back( "--print-flags" );

	auto vmCFlags = mallocScoped<const char *>( mVMFlags.size() );

	for( size_t i = 0; i < mVMFlags.size(); i++ )
		vmCFlags.get()[i] = mVMFlags[i].c_str();

	// setting VM startup options
	bool success = Dart_SetVMFlags( mVMFlags.size(), vmCFlags.get() );
	CI_VERIFY( success );

	success = Dart_Initialize(	Script::createIsolateCallback,
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

void VM::loadCinderDartLib()
{
	string script = getCinderDartScript();
	Dart_Handle source = toDart( script );
	CIDART_CHECK( source );

	Dart_Handle cinderDartLib = Dart_LoadLibrary( toDart( "cinder.dart" ), source, 0, 0 );
	if( Dart_IsError( cinderDartLib ) ) {
		throw DartException( string( "failed to load cinder.dart, error message: " ) + Dart_GetError( cinderDartLib ) );
	}

	CIDART_CHECK( Dart_SetNativeResolver( cinderDartLib, Script::resolveNameHandler, NULL ) );

	// finalize any scripts loaded, needs to be done before the libs can be looked up and modified below
	CIDART_CHECK( Dart_FinalizeLoading( false ) );

	// swap in custom _printClosure to enable print() in dart
	Dart_Handle internalLib = Dart_LookupLibrary( toDart( "dart:_internal" ) );
	CIDART_CHECK( internalLib );
	Dart_Handle print = Dart_GetField( cinderDartLib, toDart( "_printClosure" ) );
	CIDART_CHECK( print );
	CIDART_CHECK( Dart_SetField( internalLib, toDart( "_printClosure" ), print ) );
}

// static
const char* VM::getVersionString()
{
	return Dart_VersionString();
}

string VM::getCinderDartScript()
{
	if( mCinderDartScriptDataSource )
		return loadString( mCinderDartScriptDataSource );

	for( auto dirIt = mImportSearchDirectories.rbegin(); dirIt != mImportSearchDirectories.rend(); ++dirIt ) {
		auto cinderDartPath = *dirIt / "cinder.dart";
		if( fs::exists( cinderDartPath ) )
			return loadString( loadFile( cinderDartPath ) );
	}

	throw DartException( "no provided cinder.dart script file" );
}

DataSourceRef VM::getSnapShot()
{
	if( ! mSnapshot ) {
		if( ! mSnapshotPath.empty() ) {
			mSnapshot = loadFile( mSnapshotPath );
		}
#if defined( CINDER_COCOA )
		else {
			mSnapshot = app::loadResource( "snapshot_gen.bin" );
		}
#endif
	}

	return mSnapshot;
}

void VM::addImportDirectoryImpl( const fs::path &directory )
{
	if( ! fs::is_directory( directory ) ) {
		CI_LOG_E( "Not a directory: " << directory );
		return;
	}

	fs::path dirCanonical = fs::canonical( directory );
	auto it = find( mImportSearchDirectories.begin(), mImportSearchDirectories.end(), dirCanonical );
	if( it == mImportSearchDirectories.end() )
		mImportSearchDirectories.push_back( dirCanonical );
}

void VM::removeImportDirectoryImpl( const fs::path &directory )
{
	fs::path dirCanonical = fs::canonical( directory );
	mImportSearchDirectories.erase( remove( mImportSearchDirectories.begin(), mImportSearchDirectories.end(), dirCanonical ), mImportSearchDirectories.end() );
}

// ----------------------------------------------------------------------------------------------------
// Dart Callbacks
// ----------------------------------------------------------------------------------------------------

// static
bool VM::interruptIsolateCallback()
{
	CI_LOG_V( "continuing.." );
	return true;
}

// static
void VM::unhandledExceptionCallback( Dart_Handle error )
{
	 CI_LOG_E( Dart_GetError( error ) );
}

// static
void VM::shutdownIsolateCallback( void *callbackData )
{
	CI_LOG_V( "bang" );
}

// file callbacks have been copied verbatum from included sample... plus verbose logging. don't event know yet if we need them
// static
void* VM::openFileCallback(const char* name, bool write)
{
	CI_LOG_V( "name: " << name << ", write mode: " << boolalpha << write << dec );
	
	return fopen(name, write ? "w" : "r");
}

// static
void VM::readFileCallback(const uint8_t** data, intptr_t* fileLength, void* stream )
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
void VM::writeFileCallback(const void* data, intptr_t length, void* file)
{
	CI_LOG_V( "bang" );

	fwrite(data, 1, length, reinterpret_cast<FILE*>(file));
}

// static
void VM::closeFileCallback(void* file)
{
	CI_LOG_V( "bang" );

	fclose(reinterpret_cast<FILE*>(file));
}

} // namespace cidart
