#pragma once

#include "dart_api.h"

#include <vector>

namespace cinderdart {

	class CinderDart {
	public:
		CinderDart();

	private:
		Dart_Isolate mIsolate;

		std::vector<std::string> mVMFlags;
	};
} // namespace cinderdart