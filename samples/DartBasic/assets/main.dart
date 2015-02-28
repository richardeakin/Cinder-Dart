import "cinder.dart" as ci;

void customCallback( String message ) => ci.callNative1( "customCallback", message );

void main()
{
	print( "hello dart." );

	var seconds = 60 * 60 * 24 * 365.2563;
	print( "seconds in a year: $seconds" );

	customCallback( "You can edit this in main.dart and hit 'r' to reload." );
}
