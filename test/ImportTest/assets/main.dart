
import 'dart:math' as math;
import 'package:vector_math/vector_math.dart';
import 'cinder.dart';

void main() {
  print("hello dart.");
  
  var radius = math.exp( 5 );

  Vector3 x = new Vector3( 1.0, 2.0, 3.0 );
  Vector3 y = new Vector3( 2.0, 3.0, 4.0 );

  Vector3 xy = x + y;

  print( "xy: ${xy}" );

  var m = {
    'radius' : radius,
    'segments' : 6,
    'color' : new Color( 0.0, 0.3, 0.75, 1.0 ),
    'rotationRate' : 8.0,
	'someVec3' : xy
  };

  toCinder( m );
} 
 