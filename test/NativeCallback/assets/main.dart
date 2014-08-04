import 'cinder';

void blarg() native "blarg";

void main()
{
	print( "NativeCallback test." );
	blarg();
}
