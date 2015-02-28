// NOTES: This sample demonstrates how to use pub packaged dependencies, particularly vector_math
//
// First, you must get the dependency using pub (distributed with the dart editor, see www.dartlang.org)
// You do this by cd'ing into the assets folder and running 'pub get'. On windows, you need to use
// cmd.exe. 

#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Timeline.h"
#include "cinder/System.h"
#include "cinder/Camera.h"

#if CINDER_VERSION >= 807
	#include "cinder/app/RendererGl.h"
	#include "cinder/Log.h"
#endif

#include "cidart/VM.h"
#include "cidart/Script.h"
#include "cidart/Types.h"
#include "cidart/Debug.h"

#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ImportPackageApp : public AppNative {
  public:
	void setup() override;
	void resize() override;
	void keyDown( KeyEvent event ) override;
	void draw() override;

	void loadScript();
	void receiveMap(  const cidart::InfoMap& map );

	cidart::ScriptRef	mScript;
	vec3				mSphereCenter;
	float				mSphereRadius;
	CameraPersp			mCam;
};

void ImportPackageApp::setup()
{
	cidart::VM::setCinderDartScriptDataSource( loadResource( CIDART_RES_CINDER_DART ) );
	cidart::VM::setSnapshotBinDataSource( loadResource( CIDART_RES_SNAPSHOT_BIN ) );

	loadScript();

	gl::enableDepthWrite();
	gl::enableDepthRead();
}

void ImportPackageApp::loadScript()
{
	auto opts = cidart::Script::Options();

	opts.native( "submitSphere",
		 [this] ( Dart_NativeArguments args ) {
			 mSphereCenter = cidart::getArg<vec3>( args, 1 );
			 mSphereRadius = cidart::getArg<float>( args, 2 );
		 }
	);

	opts.native( "submitCamLookAt",
		[this] ( Dart_NativeArguments args ) {
			vec3 eye = cidart::getArg<vec3>( args, 1 );
			vec3 target = cidart::getArg<vec3>( args, 2 );
			mCam.lookAt( eye, target );
		}
	);

	try {
		mScript = cidart::Script::create( loadAsset( "main.dart" ), opts );
	}
	catch( Exception &exc ) {
		CI_LOG_E( "exception of type: " << System::demangleTypeName( typeid( exc ).name() ) << ", what: " << exc.what() );
		CI_LOG_E( "For this sample to work, make sure to run 'pub get' from the assets folder." );
	}
}


void ImportPackageApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'r') {
		CI_LOG_I( "reload." );
		loadScript();
	}
}

void ImportPackageApp::resize()
{
	mCam.setPerspective( 60, getWindowAspectRatio(), 1, 1000 );
}

void ImportPackageApp::draw()
{
	gl::clear();

	gl::setMatrices( mCam );

	gl::drawCoordinateFrame( 10, 2, 1 );

	gl::color( 0.9f, 0.9f, 0 );
	gl::drawSphere( mSphereCenter, mSphereRadius, 30 );
}

CINDER_APP_NATIVE( ImportPackageApp, RendererGl )
