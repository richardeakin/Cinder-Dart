#pragma once

#include "cinder/DataSource.h"

#include "dart_api.h"

#include <memory>
#include <vector>
#include <map>

namespace cinderdart {

	class CinderDart {
	public:

		typedef std::map<std::string, Dart_NativeFunction> NativeFunctionMap;

		CinderDart();

		void loadScript( ci::DataSourceRef script );
		void invoke( const std::string &functionName, int argc = 0, Dart_Handle *args = nullptr );

		NativeFunctionMap& getFunctionMap()	{ return mNativeFunctionMap; }

	private:

		Dart_Isolate mIsolate;
		std::vector<std::string> mVMFlags;

		NativeFunctionMap mNativeFunctionMap;

	};
} // namespace cinderdart