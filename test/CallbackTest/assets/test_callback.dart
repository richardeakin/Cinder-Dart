import 'cinder.dart' as ci;

// proxy to allow it to be hooked up to a std::function:
void myCallback( String message ) => ci.callNative1( "myCallback", message );

// routes to a Dart_NativeFunction directly
// void myCallback( String message ) native "myCallback";

void main()
{
	print("1");
	myCallback( "Hey Joe" );
	print("2");
	myCallback( "Hey Shmoe" );
	print("3");
}
