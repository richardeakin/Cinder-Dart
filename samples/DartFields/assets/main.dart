import 'cinder.dart';

class MovingRect {
	Vec2	pos = new Vec2( 50, 50 );
	Vec2	size = new Vec2( 20, 20 );
	Color 	color = new Color( 1, 1, 1 );
	num   	speed = 1;

	void submit() native "MovingRect::submit";
}


void main()
{
	print( "hello dart." );

	var mr = new MovingRect();

	mr.submit();
}
