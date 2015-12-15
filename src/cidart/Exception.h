// Copyright (c) 2015, Richard Eakin and the Dart project authors.
// Use of this source code (and the Dart VM) is governed by a
// BSD-style license that can be found in the LICENSE.txt file.

#pragma once

#include "cinder/Exception.h"

#include "include/dart_api.h"

namespace cidart {

//! Exception type thrown for errors originating in Cinder-Dart
class DartException : public cinder::Exception {
  public:
	DartException( const std::string &descr ) : mDescription( descr )	{}
	virtual const char* what() const throw()	{ return mDescription.c_str(); }
  protected:
	std::string mDescription;
};

//! Throws an exception into dart. If unhandled, will cause a cidart::DartException with \a description.
void	throwException( const std::string &description );
//! Throws an exception into dart if \a handle is an error.
void	throwIfError( Dart_Handle handle, const std::string &description = "" );

} // namespace cidart
