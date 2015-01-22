
import 'dart:math' as math;
import 'package:vector_math/vector_math.dart';

void submitSphere( Vector3 origin, num radius  ) native "submitSphere";
void submitCamLookAt( Vector3 eyePoint, Vector3 target  ) native "submitCamLookAt";

void main()
{
    // Using dart's built-in math functions   
    var radius = math.exp( 1.5 );

    // Using vector_math library (https://github.com/johnmccutchan/vector_math)
    Vector3 center = new Vector3( -3.0, 2.5, 2.0 );
    center *= 10.0;

    print( "submitting sphere center: ${center}, radius: ${radius}" );

    submitSphere( center, radius );
    submitCamLookAt( new Vector3( 0.0, 20.0, 100.0 ), new Vector3.zero() );
} 
 