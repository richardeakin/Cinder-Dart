import 'cinder.dart' as ci;

// proxy to allow it to be hooked up to a std::function:
void incrStdFunction( int i ) => ci.callNative1( "incrStdFunction", i );

// routes to a Dart_NativeFunction directly
void incrNative( int i ) native "incrNative";

void runIncrStdFunction()
{
	for( int i = 0; i < 1000000; i++ )
		incrStdFunction( 1 );
}

void runIncrNative()
{
	for( int i = 0; i < 1000000; i++ )
		incrNative( 1 );
}

void main()
{
}
