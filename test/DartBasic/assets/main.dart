import 'cinder.dart';

enum Fruit {
	ORANGE,
	APPLE,
	BANANA
}

void main()
{
	print( "index of BANANA: ${Fruit.BANANA.index}" );

	var info = {
		'radius' : 200.0,
		'segments' : 5,
		'color' : new Color( 1.0, 0.3, 0.15, 1.0 ),
		'rotationRate' : 8.0,
	 	'fruit'        : Fruit.APPLE
	};

	toCinder( info );
}
