library cinder;

get _printClosure => ( s ) {
	try {
    	console( s );
	} catch ( _ ) {
		throw( s );
	}
};

void console( String message ) native "console";

// TODO: use type safety 
void toCinder( var map ) native "toCinder";