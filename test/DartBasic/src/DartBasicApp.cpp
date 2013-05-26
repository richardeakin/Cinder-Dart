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

	cidart::CinderDart mDart;

	size_t mNumCircleSegments;
};

void DartBasicApp::setup()
{

	mDart.setMapReceiver( [this]( const cidart::DataMap& map ) {
		LOG_V << "huzzah" << endl;
		for( auto &mp : map ) {
			LOG_V << "key: " << mp.first << ", value: ";
			Dart_Handle value = mp.second;
			if( Dart_IsInteger( value ) ) {
				console() << cidart::getInt( value );
			}
			else if( Dart_IsDouble( value ) ) {
				console() << cidart::getFloat( value );
			}
			else {
				console() << "unknown type" << endl;
			}
		}

		auto segIt = map.find( "segments" );
		if( segIt != map.end() )
			mNumCircleSegments = cidart::getInt( segIt->second );
	} );

	mDart.loadScript( loadAsset( "main.dart" ) );
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


	gl::color( Color::white() );
	gl::drawSolidCircle( getWindowCenter(), 200.0f, mNumCircleSegments );
}

CINDER_APP_NATIVE( DartBasicApp, RendererGl )
