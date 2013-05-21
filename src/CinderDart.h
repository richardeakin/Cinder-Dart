#pragma once

#include "cinder/DataSource.h"

#include "dart_api.h"

#include <memory>
#include <vector>

namespace cinderdart {

	class CinderDart {
	public:
		CinderDart();

		void loadScript( ci::DataSourceRef script );
		void invoke( const std::string &functionName, int argc = 0, Dart_Handle *args = nullptr );

	private:

		Dart_Isolate mIsolate;
		std::vector<std::string> mVMFlags;
	};
} // namespace cinderdart