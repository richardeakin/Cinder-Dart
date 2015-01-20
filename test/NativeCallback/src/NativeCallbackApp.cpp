#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

#if CINDER_VERSION >= 807
	#include "cinder/app/RendererGl.h"
#endif

#include "Resources.h"

#include "cidart/VM.h"
#include "cidart/Script.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void customNativeCallback( Dart_NativeArguments arguments )
{
	console() << "customNativeCallback called. Look at cidart::toCinder for example usage." << endl;
}

class NativeCallbackApp : public AppNative {
  public:
	void setup();
	void draw();

	cidart::ScriptRef mScript;
};

void NativeCallbackApp::setup()
{
	cidart::VM::setCinderDartScriptDataSource( loadResource( CIDART_RES_CINDER_DART ) );
	cidart::VM::setSnapshotBinDataSource( loadResource( CIDART_RES_SNAPSHOT_BIN ) );

	auto opts = cidart::Script::Options().native( "blarg", customNativeCallback );
	mScript = cidart::Script::create( loadAsset( "main.dart" ), opts );
}

void NativeCallbackApp::draw()
{
	gl::clear(); 
}

CINDER_APP_NATIVE( NativeCallbackApp, RendererGl )
