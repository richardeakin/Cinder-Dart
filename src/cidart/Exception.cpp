// Copyright (c) 2013, Richard Eakin and the Dart project authors.
// Use of this source code (and the Dart VM) is governed by a
// BSD-style license that can be found in the LICENSE.txt file.

#include "cidart/Types.h"
#include "cidart/Debug.h"

#include <sstream>

using namespace ci;
using namespace std;

namespace cidart {

// FIXME: apparently this can't be called unless we're inside a Dart_Invoke, maybe need to move it to Script and keep track of that
// - but this isn't feasible for things like cidart::getValue(), since it doesn't know of the current script
// - possible hack solution: get the stacktrace and search for the invoke() command, if we don't find that then throw regularly
// - PROBABLE SOLUTION: build dart runtime with exceptions enabled and throw regular C++ exception
void throwException( const std::string &description )
{
	cidart::DartScope enterScope;

	Dart_Handle exceptionHandle = toDart( description );
	CIDART_CHECK( Dart_ThrowException( exceptionHandle ) );
}

void throwIfError( Dart_Handle handle, const std::string &description )
{
	if( Dart_IsError( handle ) ) {
		throwException( description + ", description: " + Dart_GetError( handle ) );
	}
}

} // namespace cidart
