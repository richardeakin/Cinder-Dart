#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class DartBasicTestApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void DartBasicTestApp::setup()
{
}

void DartBasicTestApp::mouseDown( MouseEvent event )
{
}

void DartBasicTestApp::update()
{
}

void DartBasicTestApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( DartBasicTestApp, RendererGl )
