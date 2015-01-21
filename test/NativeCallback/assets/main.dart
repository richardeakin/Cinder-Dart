
void customCallback( String message ) native "customCallback";

void main()
{
	customCallback( "A string message from the script." );
}
