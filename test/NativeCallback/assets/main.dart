import 'cinder';

void blarg() native "blarg";

bool isMap(Object object) native "IsMapMezoni";

void main() {
  	print('Mezoni Reflection example:');
	var map = {};

	bool b = isMap( map );	
	// print('${map.runtimeType} is map: ${isMap(map)}');
	// var list = [];
	// print('${list.runtimeType} is map: ${isMap(list)}');
	// var hashmap = new HashMap();
	// print('${hashmap.runtimeType} is map: ${isMap(hashmap)}');
} 
 