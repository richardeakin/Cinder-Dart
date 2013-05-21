#pragma once

#include "cinder/app/App.h"

#define LOG_V ci::app::console() << __func__ << " | "
#define LOG_E LOG_V << __LINE__ << " | " << " *** ERROR *** : "


#include <boost/assert.hpp>

#define CI_ASSERT(expr) BOOST_ASSERT(expr)

#include "dart_api.h"

#define CHECK_DART(result)							\
if (Dart_IsError(result)) {							\
	LOG_E << Dart_GetError(result) << std::endl;	\
	CI_ASSERT( 0 );									\
}

