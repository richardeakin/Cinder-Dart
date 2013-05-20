#include "CinderDart.h"
#include "debug.h"

using namespace std;

namespace cinderdart {

CinderDart::CinderDart()
{
	mVMFlags.push_back( "--enable-checked-mode" );
//	mVMFlags.push_back( "--print-flags" );

	const char **vmCFlags = (const char **)malloc( mVMFlags.size() * sizeof( const char * ) );
	for( size_t i = 0; i < mVMFlags.size(); i++ )
		vmCFlags[i] = mVMFlags[i].c_str();

	LOG_V << "Setting VM Options" << endl;
	bool success = Dart_SetVMFlags( mVMFlags.size(), vmCFlags );
	CI_ASSERT( success );
	free( vmCFlags );

	// TODO NEXT: Dart_Initialize
}

} // namespace cinderdart