library cinder;

get _printClosure => ( s ) {
  try {
    console( s );
   } catch ( _ ) {
     throw( s );
   }
 };

 void console( String what ) native "console";
