
void customCallback( String message ) native "customCallback";

void main()
{
	customCallback( "You can edit this in main.dart and hit 'r' to reload." );
}
