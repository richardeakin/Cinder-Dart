#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

#include "CinderDart.h"
#include "DartTypes.h"
#include "debug.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class DartBasicApp : public AppNative {
  public:
	void setup();
	void keyDown( KeyEvent event );
	void update();
	void draw();

	void receiveMap(  const cidart::DataMap& map );

	cidart::CinderDart mDart;

	size_t mNumCircleSegments;
	ColorA mCircleColor;
};

void DartBasicApp::setup()
{
	mNumCircleSegments = 3;
	mCircleColor = ColorA::white();

	mDart.setMapReceiver( bind( &DartBasicApp::receiveMap, this, std::_1 ) );
	mDart.loadScript( loadAsset( "main.dart" ) );
}

void DartBasicApp::receiveMap( const cidart::DataMap& map )
{
	LOG_V << "huzzah" << endl;
	for( auto &mp : map ) {
		LOG_V << "key: " << mp.first << ", value type: " << cidart::getClassName( mp.second ) << endl;
	}

	auto segIt = map.find( "segments" );
	if( segIt != map.end() )
		mNumCircleSegments = cidart::getInt( segIt->second );

	auto colorIt = map.find( "color" );
	if( colorIt != map.end() ) {
		if( cidart::isColor( colorIt->second ) )
			mCircleColor = cidart::getColor( colorIt->second );
		else
			LOG_E << "expected value for key 'color' to be of type Color." << endl;
	}

}

void DartBasicApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'r') {
		LOG_V << "reload." << endl;
		mDart.loadScript( loadAsset( "main.dart" ) ); // TODO: add CinderDart::reload
	}
}

void DartBasicApp::update()
{
}

void DartBasicApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );


	gl::color( mCircleColor );
	gl::drawSolidCircle( getWindowCenter(), 200.0f, mNumCircleSegments );
}

CINDER_APP_NATIVE( DartBasicApp, RendererGl )
