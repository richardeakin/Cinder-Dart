#pragma once

#include "cinder/DataSource.h"
#include "cinder/Function.h"

#include "dart_api.h"

#include <memory>
#include <vector>
#include <map>

#include <boost/any.hpp>

namespace cinderdart {

	typedef std::map<std::string, boost::any> DataMap;

	class CinderDart {
	public:

		typedef std::map<std::string, Dart_NativeFunction> NativeFunctionMap;
		typedef std::function<void( const DataMap& )>	ReceiveMapCallback;

		CinderDart();

		void loadScript( ci::DataSourceRef script );
		void invoke( const std::string &functionName, int argc = 0, Dart_Handle *args = nullptr );

		void setMapReceiver( const ReceiveMapCallback& callback )	{ mReceiveMapCallback = callback; }
	private:

		Dart_Isolate mIsolate;
		std::vector<std::string> mVMFlags;

		NativeFunctionMap mNativeFunctionMap;

		ReceiveMapCallback mReceiveMapCallback;

		friend void toCinder( Dart_NativeArguments arguments );
		friend Dart_NativeFunction resolveName( Dart_Handle handle, int argc );
	};
} // namespace cinderdart