#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/System.h"

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

class CallbackTestApp : public AppNative {
  public:
	void setup() override;
	void keyDown( KeyEvent event ) override;
	void draw() override;

	void loadScript();

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

void CallbackTestApp::loadScript()
{
	auto opts = cidart::Script::Options().native( "myCallback",
				[this] ( Dart_NativeArguments args ) {
					mScriptMessage = cidart::getArg<string>( args, 0 );
					CI_LOG_I( "mScriptMessage: " << mScriptMessage );
				}
	);

	try {
		mScript = cidart::Script::create( loadAsset( "test_callback.dart" ), opts );
	}
	catch( Exception &exc ) {
		CI_LOG_E( "exception of type: " << System::demangleTypeName( typeid( exc ).name() ) << ", what: " << exc.what() );
	}
}

void CallbackTestApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'r' )
		loadScript();
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
