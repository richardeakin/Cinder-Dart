
void myCallback( String message ) native "myCallback";

void main()
{
	print("1");
	myCallback( "Hey Hey" );
	print("2");
	myCallback( "Hey Hey" );
	print("3");
}
