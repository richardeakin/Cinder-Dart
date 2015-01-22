import 'cinder.dart';

import 'dart:math' as math;

math.Random rand = new math.Random();

class BreathingRect {
	Vec2	pos = new Vec2( 50, 50 );
	Vec2	size = new Vec2( 20, 20 );
	Color 	color = new Color( 1, 1, 1 );
	num   	speed = 1;
	num     seed = 0;
}

// This is invoked from the App's mouseDown()
BreathingRect makeBreathingRect()
{
	var result = new BreathingRect();

	result.size.x *= 2.0 + rand.nextDouble() * 2.0;
	result.size.y *= 2.0 + rand.nextDouble() * 2.0;
	result.color.r = rand.nextDouble();
	result.color.b = 0.5 + rand.nextDouble() * 0.5;
	result.color.a = 0.5 + rand.nextDouble() * 0.5;
	result.speed = 0.01 + rand.nextDouble() * 0.22;
	result.seed = rand.nextDouble();

	return result;
}

void main()
{
	print( "Click somewhere in the window." );
}
