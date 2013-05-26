#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Timeline.h"

#include "cidart/DartVM.h"
#include "cidart/DartTypes.h"
#include "cidart/debug.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class DartBasicApp : public AppNative {
  public:
	void setup();
	void keyDown( KeyEvent event );
	void draw();

	void receiveMap(  const cidart::DataMap& map );

	cidart::DartVM mDart;

	size_t mNumCircleSegments;
	ColorA mCircleColor;
	float mCircleRadius, mRotationRate;
	Anim<float> mRotation;
};

void DartBasicApp::setup()
{
	// these values will be updated from main.dart:
	mCircleRadius = 1.0f;
	mNumCircleSegments = 3;
	mCircleColor = ColorA::white();
	mRotationRate = 2.0f;
	mRotation = 0;

	mDart.setMapReceiver( bind( &DartBasicApp::receiveMap, this, std::_1 ) );
	mDart.loadScript( loadAsset( "main.dart" ) );

}

void DartBasicApp::receiveMap( const cidart::DataMap& map )
{
	LOG_V << "huzzah" << endl;
	for( auto &mp : map ) {
		LOG_V << "key: " << mp.first << ", value type: " << cidart::getClassName( mp.second ) << endl;
	}

	auto radiusIt = map.find( "radius" );
	if( radiusIt != map.end() )
		mCircleRadius = cidart::getFloat( radiusIt->second );

	auto segIt = map.find( "segments" );
	if( segIt != map.end() )
		mNumCircleSegments = cidart::getInt( segIt->second );

	auto colorIt = map.find( "color" );
	if( colorIt != map.end() )
		mCircleColor = cidart::getColor( colorIt->second );

	auto rotationRateIt = map.find( "rotationRate" );
	if( rotationRateIt != map.end() ) {
		mRotationRate = cidart::getFloat( rotationRateIt->second );
		timeline().apply( &mRotation, mRotation + 360.0f, mRotationRate ).loop();
	}
}

void DartBasicApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'r') {
		LOG_V << "reload." << endl;
		mDart.loadScript( loadAsset( "main.dart" ) ); // TODO: add DartVM::reload
	}
}

void DartBasicApp::draw()
{
	gl::clear();

	gl::pushMatrices();
		gl::translate( getWindowCenter() );
		gl::rotate( mRotation );
		gl::color( mCircleColor );
		gl::drawSolidCircle( Vec2f::zero(), mCircleRadius, mNumCircleSegments );
	gl::popMatrices();
}

CINDER_APP_NATIVE( DartBasicApp, RendererGl )
