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

  // FIXME: these methods with optional params work with dart editor / try.dartlang.org,
  // but they give 'incorrect number of arguments' here.
  // Color( [this.r = 0.0, this.g = 0.0, this.b = 0.0, this.a = 1.0] );
  // Color( this.r = 0.0, this.g = 0.0, this.b = 0.0, [this.a = 1.0] );

  Color( this.r, this.g, this.b, this.a );
 }