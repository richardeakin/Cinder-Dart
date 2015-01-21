#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/Log.h"
#include "cinder/System.h"

#if CINDER_VERSION >= 807
	#include "cinder/app/RendererGl.h"
#endif

#include "Resources.h"

#include "cidart/VM.h"
#include "cidart/Script.h"
#include "cidart/Types.h"

using namespace ci;
using namespace ci::app;
using namespace std;

std::string sScriptMessage;

void customNativeCallback( Dart_NativeArguments args )
{
	Dart_Handle firstArg = Dart_GetNativeArgument( args, 0 );
	sScriptMessage = cidart::getValue<string>( firstArg );
}

class NativeCallbackApp : public AppNative {
  public:
	void setup() override;
	void keyDown( KeyEvent event ) override;
	void draw() override;

	void loadScript();

	cidart::ScriptRef	mScript;
	gl::TextureFontRef	mTextureFont;
};

void NativeCallbackApp::setup()
{
	cidart::VM::setCinderDartScriptDataSource( loadResource( CIDART_RES_CINDER_DART ) );
	cidart::VM::setSnapshotBinDataSource( loadResource( CIDART_RES_SNAPSHOT_BIN ) );
	loadScript();

	// setup rendering
	mTextureFont = gl::TextureFont::create( Font( "Arial", 22 ) );
	gl::enableAlphaBlending();
}

void NativeCallbackApp::loadScript()
{
	try {
		auto opts = cidart::Script::Options().native( "customCallback", customNativeCallback );
		mScript = cidart::Script::create( loadAsset( "main.dart" ), opts );
	}
	catch( Exception &exc ) {
		CI_LOG_E( "exception of type: " << System::demangleTypeName( typeid( exc ).name() ) << ", what: " << exc.what() );
	}
}

void NativeCallbackApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'r' )
		loadScript();
}

void NativeCallbackApp::draw()
{
	gl::clear();

	if( ! sScriptMessage.empty() ) {
		gl::color( Color( 0, 1, 1 ) );
		mTextureFont->drawString( sScriptMessage, vec2( 10, getWindowCenter().y ) );
	}
}

CINDER_APP_NATIVE( NativeCallbackApp, RendererGl )
