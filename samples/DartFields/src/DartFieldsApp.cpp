#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"

#include "cidart/VM.h"
#include "cidart/Script.h"
#include "cidart/Types.h"

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
	ci::gl::BatchRef			mBatchRect;
};

void DartFieldsApp::setup()
{
	cidart::VM::setCinderDartScriptDataSource( loadResource( CIDART_RES_CINDER_DART ) );
	cidart::VM::setSnapshotBinDataSource( loadResource( CIDART_RES_SNAPSHOT_BIN ) );
	loadScript();

	// setup rendering
	mBatchRect = gl::Batch::create( geom::Rect(), gl::getStockShader( gl::ShaderDef().color() ) );
	gl::enableAlphaBlending();
}

void DartFieldsApp::loadScript()
{
	try {
		mScript = cidart::Script::create( loadAsset( "main.dart" ) );
	}
	catch( Exception &exc ) {
		CI_LOG_EXCEPTION( "failed to load script", exc );
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

		gl::ScopedModelMatrix modelScope;
		gl::translate( br.pos );
		gl::scale( br.size * br.breath );

		mBatchRect->draw();
	}
}

CINDER_APP( DartFieldsApp, RendererGl( RendererGl::Options().msaa( 8 ) ) )
