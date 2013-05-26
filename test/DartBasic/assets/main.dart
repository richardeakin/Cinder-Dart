import 'cinder';

void main() {
  print("hello dart.");
  
  var answer = 23 * 4 / 2;
  print( "the answer is $answer" );

  // TODO: report weird printout error message..
  //Color c = new Color( 1.0, 0.0, 0.0, 1.0 );
  // print( "r: ${c.r}, g: ${c.g}, b: ${c.b}, a: ${c.a");

  var m = {
    'a' : 1,
    'b' : 2.0,
    'color' : new Color( 1.0, 0.3, 0.15, 1.0 ),
    'segments' : 5
  };

  toCinder( m );
} 
 