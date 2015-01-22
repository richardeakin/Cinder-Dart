import 'cinder.dart';

enum Fruit {
	ORANGE,
	APPLE,
	BANANA
}

void main()
{
  print("hello dart.");
  
  var answer = 23 * 4 / 2;
  print( "the answer is $answer" );

	print( "index of BANANA: ${Fruit.BANANA.index}" );

//  Color c = new Color( 1.0, 0.0, 0.0 );
//  print( "r: ${c.r}, g: ${c.g}, b: ${c.b}, a: ${c.a}");

  var m = {
    'radius' : 200.0,
    'segments' : 5,
    'color' : new Color( 1.0, 0.3, 0.15, 1.0 ),
    'rotationRate' : 8.0,
	'fruit'        : Fruit.APPLE
  };

  toCinder( m );
}
