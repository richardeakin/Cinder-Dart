#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/System.h"
#include "cinder/Timer.h"

#if CINDER_VERSION >= 807
	#include "cinder/app/RendererGl.h"
	#include "cinder/Log.h"
#else
	#include "cidart/Debug.h"
#endif

#include "Resources.h"

#include "cidart/VM.h"
#include "cidart/Script.h"
#include "cidart/Types.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void nativeFunctionCallback( Dart_NativeArguments args )
{
	auto message = cidart::getArg<string>( args, 0 );
	CI_LOG_I( "message: " << message );
}

class CallbackTestApp : public AppNative {
  public:
	void setup() override;
	void keyDown( KeyEvent event ) override;
	void draw() override;
	void memberFunctionCallback( Dart_NativeArguments args );

	void loadScript();
	void runBenchmarks();

	cidart::ScriptRef	mScript;
	std::string			mScriptMessage;
	gl::TextureFontRef	mTextureFont;
};

void CallbackTestApp::setup()
{
	cidart::VM::setCinderDartScriptDataSource( loadResource( CIDART_RES_CINDER_DART ) );
	cidart::VM::setSnapshotBinDataSource( loadResource( CIDART_RES_SNAPSHOT_BIN ) );
	loadScript();

	// setup rendering
	mTextureFont = gl::TextureFont::create( Font( "Arial", 22 ) );
	gl::enableAlphaBlending();
}

void CallbackTestApp::memberFunctionCallback( Dart_NativeArguments args )
{
	mScriptMessage = cidart::getArg<string>( args, 1 ); // first arg is native method id
	CI_LOG_I( "mScriptMessage: " << mScriptMessage );
}

void CallbackTestApp::loadScript()
{
	auto opts = cidart::Script::Options();
	opts.native( "myCallback", std::bind( &CallbackTestApp::memberFunctionCallback, this, placeholders::_1 ) );
//	opts.native( "myCallback", nativeFunctionCallback );

	try {
		mScript = cidart::Script::create( loadAsset( "test_callback.dart" ), opts );
	}
	catch( Exception &exc ) {
		CI_LOG_E( "exception of type: " << System::demangleTypeName( typeid( exc ).name() ) << ", what: " << exc.what() );
	}
}

void incrNativeHandler( Dart_NativeArguments args )
{
	static uint64_t sIncr = 0;
	sIncr += cidart::getArg<int>( args, 0 );
}

void CallbackTestApp::runBenchmarks()
{
	Timer timer;

	try {
		uint64_t incr = 0;
		auto opts = cidart::Script::Options();
		opts.native( "incrStdFunction", [&incr]( Dart_NativeArguments args ) {
			incr += cidart::getArg<int>( args, 1 );
		});
		opts.native( "incrNative", incrNativeHandler );

		timer.start();

		CI_LOG_I( "running benchmarks..." );
		auto script = cidart::Script::create( loadAsset( "benchmark_callbacks.dart" ), opts );
		console() << ".. complete, script load time (ms): " << timer.getSeconds() * 1000 << endl;

		timer.stop();
		timer.start();

		{
			cidart::DartScope enterScope;
			script->invoke( "runIncrStdFunction" );
		}

		console() << ".. std::function callback time (ms): " << timer.getSeconds() * 1000 << endl;

		timer.stop();
		timer.start();

		{
			cidart::DartScope enterScope;
			script->invoke( "runIncrNative" );
		}

		console() << ".. native callback time (ms): " << timer.getSeconds() * 1000 << endl;
	}
	catch( Exception &exc ) {
		CI_LOG_E( "exception of type: " << System::demangleTypeName( typeid( exc ).name() ) << ", what: " << exc.what() );
	}

}

void CallbackTestApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'r' )
		loadScript();
	if( event.getChar() == 'b' )
		runBenchmarks();
}

void CallbackTestApp::draw()
{
	gl::clear();

	if( ! mScriptMessage.empty() ) {
		gl::color( Color( 0, 1, 1 ) );
		mTextureFont->drawString( mScriptMessage, vec2( 10, getWindowCenter().y ) );
	}
}

CINDER_APP_NATIVE( CallbackTestApp, RendererGl )
