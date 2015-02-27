import 'cinder.dart' as ci;

// proxy to allow it to be hooked up to a std::function:
void incrStdFunction( int i ) => ci.callNative1( "incrStdFunction", i );

// routes to a Dart_NativeFunction directly
void incrNative( int i ) native "incrNative";

const int numIterations = 1000000; // must match the numbers in the dart benchmarks;

void runIncrStdFunction()
{
	for( int i = 0; i < numIterations; i++ )
		incrStdFunction( 1 );
}

void runIncrNative()
{
	for( int i = 0; i < numIterations; i++ )
		incrNative( 1 );
}

void main()
{
}
