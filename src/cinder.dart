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