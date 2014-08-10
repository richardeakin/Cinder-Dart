
import 'dart:math' as math;
import 'package:vector_math/vector_math.dart';
import 'cinder';

void main() {
  print("hello dart.");
  
  var radius = math.exp( 4 );

  Vector3 x = new Vector3( 1.0, 2.0, 3.0 );
  Vector3 y = new Vector3( 2.0, 3.0, 4.0 );

  Vector3 xy = x + y;

  print( "xy: ${xy}" );

  var m = {
    'radius' : radius,
    'segments' : 5,
    'color' : new Color( 1.0, 0.3, 0.15, 1.0 ),
    'rotationRate' : 8.0
  };

  toCinder( m );
} 
 