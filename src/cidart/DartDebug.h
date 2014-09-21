// Copyright (c) 2013, Richard Eakin and the Dart project authors.
// Use of this source code (and the Dart VM) is governed by a
// BSD-style license that can be found in the LICENSE.txt file.

#pragma once

#include "cinder/app/App.h"
#include "cinder/CinderAssert.h"

#include "include/dart_api.h"

#if( CINDER_VERSION >= 900 )

#include "cinder/Log.h"

#else

// add cinder/Log.h substitutes

#define CIDART_LOG_VERBOSE 0

#if CIDART_LOG_VERBOSE
	#define CI_LOG_V( stream )		{ ci::app::console() << __PRETTY_FUNCTION__ << " |V| " << stream << std::endl; }
#else
	#define CI_LOG_V( stream )		( (void)0 )
#endif

#define CI_LOG_I( stream )			{ ci::app::console() << __PRETTY_FUNCTION__ << " |I| " << stream << std::endl; }
#define CI_LOG_E( stream )			{ ci::app::console() << __PRETTY_FUNCTION__ << " |ERROR| " << stream << std::endl; }

#endif // ( CINDER_VERSION >= 900 )

#define CIDART_CHECK( result )						\
{													\
	if( Dart_IsError( result ) ) {					\
		CI_LOG_E( Dart_GetError( result ) );		\
		CI_ASSERT( 0 );								\
	}												\
}
