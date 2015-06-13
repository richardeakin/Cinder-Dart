#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/Timeline.h"

#include "cinder/app/RendererGl.h"
#include "cinder/Log.h"

#include "cidart/VM.h"
#include "cidart/Script.h"
#include "cidart/Types.h"
#include "cidart/Debug.h"

#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class InfoMapApp : public App {
  public:
	void setup();
	void keyDown( KeyEvent event );
	void draw();

	void loadScript();
	void receiveMap(  const cidart::InfoMap& map );

	cidart::ScriptRef mScript;

	size_t		mNumCircleSegments;
	ColorA		mCircleColor;
	float		mCircleRadius, mRotationRate;
	Anim<float> mRotation;
};

void InfoMapApp::setup()
{
	CI_LOG_I( "dart runtime version: " << cidart::VM::getVersionString() );

	// these values will be updated from main.dart:
	mCircleRadius = 1.0f;
	mNumCircleSegments = 3;
	mCircleColor = ColorA::white();
	mRotationRate = 2.0f;
	mRotation = 0;

	cidart::VM::setCinderDartScriptDataSource( loadResource( CIDART_RES_CINDER_DART ) );
	cidart::VM::setSnapshotBinDataSource( loadResource( CIDART_RES_SNAPSHOT_BIN ) );

	loadScript();
}

void InfoMapApp::loadScript()
{
	try {
		auto opts = cidart::Script::Options().mapReceiver( bind( &InfoMapApp::receiveMap, this, placeholders::_1 ) );
		mScript = cidart::Script::create( loadAsset( "main.dart" ), opts );
	}
	catch( Exception &exc ) {
		CI_LOG_EXCEPTION( "failed to load script", exc );
	}
}

void InfoMapApp::receiveMap( const cidart::InfoMap &info )
{
	CI_LOG_I( "info map received, number of elemnts: " << info.size() );
	for( auto &mp : info ) {
		CI_LOG_I( "\tkey: " << mp.first << ", value type: " << cidart::getTypeName( mp.second ) );
	}

	auto radiusIt = info.find( "radius" );
	if( radiusIt != info.end() )
		mCircleRadius = cidart::getValue<float>( radiusIt->second );

	auto segIt = info.find( "segments" );
	if( segIt != info.end() )
		mNumCircleSegments = cidart::getValue<int>( segIt->second );

	auto colorIt = info.find( "color" );
	if( colorIt != info.end() )
		mCircleColor = cidart::getValue<ColorA>( colorIt->second );

	auto rotationRateIt = info.find( "rotationRate" );
	if( rotationRateIt != info.end() ) {
		mRotationRate = cidart::getValue<float>( rotationRateIt->second );
		timeline().apply( &mRotation, mRotation + 360.0f, mRotationRate ).loop();
	}

	auto fruitIt = info.find( "fruit" );
	if( fruitIt != info.end() ) {
		Dart_Handle fruitStringHandle = cidart::callFunction( fruitIt->second, "toString", 0, nullptr );
		string fruitString = cidart::getValue<string>( fruitStringHandle );

		CI_LOG_I( "fruit enum value as string: " << fruitString );
	}
}

void InfoMapApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'r' ) {
		CI_LOG_V( "reload." );
		loadScript();
	}
}

void InfoMapApp::draw()
{
	gl::clear();

	gl::ScopedModelMatrix modelScope;

	gl::translate( getWindowCenter() );
	gl::rotate( toRadians( mRotation() ) );

	gl::color( mCircleColor );
	gl::drawSolidCircle( vec2( 0, 0 ), mCircleRadius, mNumCircleSegments );
}

CINDER_APP( InfoMapApp, RendererGl( RendererGl::Options().msaa( 8 ) ) )
