#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/System.h"

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

struct MovingRect {
	vec2	pos;
	vec2	size;
	ColorA	color;
	float   speed;
};

class DartFieldsApp : public AppNative {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void keyDown( KeyEvent event ) override;
	void draw() override;

	void loadScript();
	void receiveMovingRect( Dart_NativeArguments args );

	cidart::ScriptRef	mScript;

	std::vector<MovingRect>	mMovingRects;
};

void DartFieldsApp::setup()
{
	cidart::VM::setCinderDartScriptDataSource( loadResource( CIDART_RES_CINDER_DART ) );
	cidart::VM::setSnapshotBinDataSource( loadResource( CIDART_RES_SNAPSHOT_BIN ) );
	loadScript();

	// setup rendering
	gl::enableAlphaBlending();
}

void DartFieldsApp::loadScript()
{
	try {
		auto opts = cidart::Script::Options().native( "MovingRect::submit", bind( &DartFieldsApp::receiveMovingRect, this, placeholders::_1 ) );
		mScript = cidart::Script::create( loadAsset( "main.dart" ), opts );
	}
	catch( Exception &exc ) {
		CI_LOG_E( "exception of type: " << System::demangleTypeName( typeid( exc ).name() ) << ", what: " << exc.what() );
	}
}

// TODO: get this as a result from invoke()
void DartFieldsApp::receiveMovingRect( Dart_NativeArguments args )
{
	MovingRect mr;

	Dart_Handle handle = Dart_GetNativeArgument( args, 0 ); // 0 arg is the this argument

	mr.pos = getMousePos() - getWindowPos();
	mr.size = cidart::getField<vec2>( handle, "size" );
	mr.color = cidart::getField<ColorA>( handle, "color" );
	mr.speed = cidart::getField<float>( handle, "speed" );

	mMovingRects.push_back( mr );

	CI_LOG_I( "complete, mr.pos: " << mr.pos << ", size: " << mr.size << ", color: " << mr.color );
}

void DartFieldsApp::mouseDown( MouseEvent event )
{
	// TODO: fire off invoke
	loadScript();
}

void DartFieldsApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'r' )
		loadScript();
}

void DartFieldsApp::draw()
{
	gl::clear();

	for( const auto &mr : mMovingRects ) {
		gl::color( mr.color );

		vec2 size2 = mr.size / 2;
		Rectf rect( mr.pos.x - size2.x, mr.pos.y - size2.y, mr.pos.x + size2.x, mr.pos.y + size2.y );
		gl::drawSolidRect( rect );
	}
}

CINDER_APP_NATIVE( DartFieldsApp, RendererGl )
