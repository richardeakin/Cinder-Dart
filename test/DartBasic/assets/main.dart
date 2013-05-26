import 'cinder';

void main() {
  print("hello dart.");
  
  var answer = 23 * 4 / 2;
  print( "the answer is $answer" );

  var m = {
    'a' : 1,
    'b' : 2.0,
    'color' : [1, 0.0, 0.5],
    'segments' : 13
  };
  // print( 'm: $m' );

  toCinder( m );
} 
 