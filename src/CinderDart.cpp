#include "CinderDart.h"
#include "debug.h"
#include "Resources.h"
#include "DartTypes.h"

#include "cinder/app/App.h"
#include "cinder/Utilities.h"

// TODO NEXT: create cinder.dart package with print / setup stuff, load it from main.dart
// TODO: report required blank setup() call as bug, once this sample is public

using namespace std;
using namespace ci;

namespace cinderdart {

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


CinderDart::CinderDart()
{
	mVMFlags.push_back( "--enable-checked-mode" );
//	mVMFlags.push_back( "--print-flags" );

	const char **vmCFlags = (const char **)malloc( mVMFlags.size() * sizeof( const char * ) );
	for( size_t i = 0; i < mVMFlags.size(); i++ )
		vmCFlags[i] = mVMFlags[i].c_str();

	LOG_V << "Setting VM Options" << endl;
	bool success = Dart_SetVMFlags( mVMFlags.size(), vmCFlags );
	CI_ASSERT( success );
	free( vmCFlags );

	success = Dart_Initialize( createIsolateCallback, interruptIsolateCallback, unhandledExceptionCallback, shutdownIsolateCallback, openFileCallback, readFileCallback, writeFileCallback, closeFileCallback );
	CI_ASSERT( success );

	LOG_V << "Dart_Initialize complete." << endl;
}

void CinderDart::loadScript( ci::DataSourceRef script )
{
	const char *scriptPath = script->getFilePath().c_str();
	char *error;
	mIsolate = createIsolateCallback( scriptPath, "main", this, &error );
	if( ! mIsolate ) {
		LOG_E << "could not create isolate: " << error << endl;
		CI_ASSERT( false );
	}

	// TODO: try DartScope here and see if it messes with isolate's exit
	Dart_EnterScope();

	Dart_Handle url = Dart_NewStringFromCString( scriptPath );
	CHECK_DART( url );
	string scriptContents = loadString( script );

//	LOG_V << "script contents: " << scriptContents << endl;

	Dart_Handle source = Dart_NewStringFromCString( scriptContents.c_str() );
	CHECK_DART( source );
	CHECK_DART( Dart_LoadScript( url, source, 0, 0 ) );

	// TODO: below was in the tag handler - check if it comes up there after 'import : cinder'
	
	// apparently 'something' must be called before swapping in print,
	// else she blows up with: parser.cc:4996: error: expected: current_class().is_finalized()
//	invoke( "setup" );
//
//	Dart_Handle library = Dart_RootLibrary();
//	if ( Dart_IsNull( library ) ) {
//		LOG_E << "Unable to find root library" << endl;
//		return;
//	}
//
//	// load in our custom _printCloser, which maps back to Log
//	Dart_Handle corelib = checkError( Dart_LookupLibrary( Dart_NewStringFromCString( "dart:core" ) ) );
//	Dart_Handle print = checkError( Dart_GetField( library, Dart_NewStringFromCString( "_printClosure" ) ) );
//	checkError( Dart_SetField( corelib, Dart_NewStringFromCString( "_printClosure" ), print ) );
//
//	checkError( Dart_SetNativeResolver( library, ResolveName ) );

	// I guess main needs to be manually invoked...
	// TODO: check dartium to see how it handles this part.
	//	- maybe it is handled with Dart_RunLoop() ?
	invoke( "main" );

	Dart_ExitScope();
    Dart_ExitIsolate();
}

void CinderDart::invoke( const string &functionName, int argc, Dart_Handle* args )
{
	Dart_Handle library = Dart_RootLibrary();
	CI_ASSERT( ! Dart_IsNull( library ) );

	Dart_Handle nameHandle = Dart_NewStringFromCString( functionName.c_str() );
	Dart_Handle result = Dart_Invoke( library, nameHandle, argc, args );
	CHECK_DART( result );

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
	LOG_V << "(unahndled) url: " << url;

	if (tag == kCanonicalizeUrl) {
		LOG_V << "\tkCanonicalizeUrl" << endl;
		return urlHandle;
	}
	return nullptr;
}

} // namespace cinderdart