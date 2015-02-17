library cinder;

get _printClosure => ( s ) {
	try {
    	printNative( s );
	} catch ( _ ) {
		throw( s );
	}
};

void printNative( String message ) 			native "cidart::printNative";
void toCinder( Map<String, dynamic> data ) 	native "cidart::toCinder";

/// callNativeN methods allow the method to hook up to a std::function. On the C++ side, the first argument recieved will be 'id'.
void callNative0( String id ) native "cidart::callNative0";
void callNative1( String id, var arg1 ) native "cidart::callNative1";
void callNative2( String id, var arg1, var arg2 ) native "cidart::callNative2";
void callNative3( String id, var arg1, var arg2, var arg3 ) native "cidart::callNative3";
void callNative4( String id, var arg1, var arg2, var arg3, var arg4 ) native "cidart::callNative4";
void callNative5( String id, var arg1, var arg2, var arg3, var arg4, var arg5 ) native "cidart::callNative5";

class Color {
	num r, g, b, a;

	Color( this.r, this.g, this.b, [this.a = 1.0] );
}

class Vec2 {
	num x, y;
	
	Vec2( this.x, this.y );
}

class Vec3 {
	num x, y, z;
	
	Vec3( this.x, this.y, this.z );
}

class Rect {
	num x1, x2, y1, y2;

	Rect( this.x1, this.y1, this.x2, this.y2 );
}