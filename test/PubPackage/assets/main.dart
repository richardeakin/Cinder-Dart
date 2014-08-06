import 'cinder';

import 'dart:math' as math;
import 'package:vector_math/vector_math.dart';

void main() {
  print("hello dart.");
  
  var radius = math.exp( 4 );

  var m = {
    'radius' : radius,
    'segments' : 5,
    'color' : new Color( 1.0, 0.3, 0.15, 1.0 ),
    'rotationRate' : 8.0
  };

  toCinder( m );
} 
 