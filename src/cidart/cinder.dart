library cinder;

get _printClosure => ( s ) {
	try {
    	console( s );
	} catch ( _ ) {
		throw( s );
	}
};

void console( String message ) native "console";
void toCinder( Map<String, dynamic> data ) native "toCinder";


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