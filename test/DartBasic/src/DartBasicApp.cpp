#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Timeline.h"

#if CINDER_VERSION >= 807
	#include "cinder/app/RendererGl.h"
#endif

#include "cidart/VM.h"
#include "cidart/Script.h"
#include "cidart/Types.h"
#include "cidart/Debug.h"

#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class DartBasicApp : public AppNative {
  public:
	void setup();
	void keyDown( KeyEvent event );
	void draw();

	void loadScript();
	void receiveMap(  const cidart::DataMap& map );

	cidart::ScriptRef mScript;

	size_t		mNumCircleSegments;
	ColorA		mCircleColor;
	float		mCircleRadius, mRotationRate;
	Anim<float> mRotation;
};

void DartBasicApp::setup()
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

void DartBasicApp::loadScript()
{
	try {
		auto opts = cidart::Script::Options().mapReceiver( bind( &DartBasicApp::receiveMap, this, placeholders::_1 ) );
		mScript = cidart::Script::create( loadAsset( "main.dart" ), opts );
	}
	catch( cidart::DartException &exc ) {
		CI_LOG_E( "caught dart exception, what: " << exc.what() );
	}
}

void DartBasicApp::receiveMap( const cidart::DataMap &map )
{
	CI_LOG_I( "huzzah" );
	for( auto &mp : map ) {
		CI_LOG_I( "key: " << mp.first << ", value type: " << cidart::getTypeName( mp.second ) );
	}

	auto radiusIt = map.find( "radius" );
	if( radiusIt != map.end() )
		mCircleRadius = cidart::getValue<float>( radiusIt->second );

	auto segIt = map.find( "segments" );
	if( segIt != map.end() )
		mNumCircleSegments = cidart::getValue<int>( segIt->second );

	auto colorIt = map.find( "color" );
	if( colorIt != map.end() )
		mCircleColor = cidart::getValue<ColorA>( colorIt->second );

	auto rotationRateIt = map.find( "rotationRate" );
	if( rotationRateIt != map.end() ) {
		mRotationRate = cidart::getValue<float>( rotationRateIt->second );
		timeline().apply( &mRotation, mRotation + 360.0f, mRotationRate ).loop();
	}
}

void DartBasicApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'r' ) {
		CI_LOG_V( "reload." );
		loadScript();
	}
}

void DartBasicApp::draw()
{
	gl::clear();

	gl::pushMatrices();
		gl::translate( getWindowCenter() );
#if CINDER_VERSION >= 807
		gl::rotate( toRadians( mRotation() ) );
#else
		gl::rotate( mRotation );
#endif
		gl::color( mCircleColor );
		gl::drawSolidCircle( vec2( 0, 0 ), mCircleRadius, mNumCircleSegments );
	gl::popMatrices();
}

CINDER_APP_NATIVE( DartBasicApp, RendererGl )
