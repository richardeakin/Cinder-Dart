#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

#include "CinderDart.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class DartBasicApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();

	cinderdart::CinderDart mDart;
};

void DartBasicApp::setup()
{
}

void DartBasicApp::mouseDown( MouseEvent event )
{
}

void DartBasicApp::update()
{
}

void DartBasicApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( DartBasicApp, RendererGl )
