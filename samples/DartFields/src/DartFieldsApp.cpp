#include "cinder/app/App.h"
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

struct BreathingRect {
	vec2	pos;
	vec2	size;
	ColorA	color;
	float   speed;
	float	breath;
	float   seed;
};

class DartFieldsApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void mouseDrag( MouseEvent event ) override;
	void keyDown( KeyEvent event ) override;
	void update() override;
	void draw() override;

	void loadScript();

	cidart::ScriptRef	mScript;

	std::vector<BreathingRect>	mBreathingRects;
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
		mScript = cidart::Script::create( loadAsset( "main.dart" ) );
	}
	catch( Exception &exc ) {
		CI_LOG_E( "exception of type: " << System::demangleTypeName( typeid( exc ).name() ) << ", what: " << exc.what() );
	}
}

void DartFieldsApp::mouseDown( MouseEvent event )
{
	// Before calling into the script, you must enter into 'dart scope'.
	// This way, you will have access to the handle it returns, and all resources will be freed at the end of the function.
	cidart::DartScope  enterScope;

	Dart_Handle handle = mScript->invoke( "makeBreathingRect" );

	BreathingRect br;
	br.pos = getMousePos() - getWindowPos();
	br.breath = 0;
	br.size = cidart::getField<vec2>( handle, "size" );
	br.color = cidart::getField<ColorA>( handle, "color" );
	br.speed = cidart::getField<float>( handle, "speed" );
	br.seed = cidart::getField<float>( handle, "seed" );

	mBreathingRects.push_back( br );
}

void DartFieldsApp::mouseDrag( MouseEvent event )
{
	mouseDown( event );
}

void DartFieldsApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'r' ) // reload script
		loadScript();
	if( event.getChar() == 'c' ) // clear existing objects
		mBreathingRects.clear();
}

void DartFieldsApp::update()
{
	for( auto &br : mBreathingRects )
		br.breath = 0.7f + sinf( br.speed + getElapsedSeconds() + br.seed * 1000 ) * 0.3f;
}

void DartFieldsApp::draw()
{
	gl::clear();

	for( const auto &br : mBreathingRects ) {
		gl::color( br.color );

		vec2 size2 = ( br.size * br.breath ) / 2.0f;
		Rectf rect( br.pos.x - size2.x, br.pos.y - size2.y, br.pos.x + size2.x, br.pos.y + size2.y );
		gl::drawSolidRect( rect );
	}
}

CINDER_APP( DartFieldsApp, RendererGl )
