/*
  Arduino.h - Main include file for the Arduino SDK
  Copyright (c) 2005-2013 Arduino Team.  All right reserved.
  Copyright (c) 2019 MCCI Corporation. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef Arduino_h
#define Arduino_h

#ifndef GCC_VERSION
#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)
#endif
#if GCC_VERSION < 60300
#error "GCC version 6.3 or higher is required"
#endif

///
/// \brief create a library semantic version
///
/// \param [in] major Major version (per [semver2.0](https://semver.org/spec/v2.0.0.html))
/// \param [in] major Major version (per [semver2.0](https://semver.org/spec/v2.0.0.html))
/// \param [in] major Major version (per [semver2.0](https://semver.org/spec/v2.0.0.html))
/// \param [in] local Pre-release version (per [semver2.0](https://semver.org/spec/v2.0.0.html))
///
/// \note local "0" is *greater than* any local non-zero; use
///     \ref _mcci_arduino_version_compare_ge() and so forth to compare relative versions.
///
#define _mcci_arduino_version_calc(major, minor, patch, local)	\
	(((major) << 24u) | ((minor) << 16u) | ((patch) << 8u) | (local))

///
/// \brief current library semantic version
///
/// \note A version with \c local == 0 is *greater than* any version that is otherwise
///     the same but has \c local non-zero; use
///     \ref _mcci_arduino_version_calc() to compare relative versions.
///
#define	_mcci_arduino_version	\
  _mcci_arduino_version_calc(3, 0, 0, 3)	/* v3.0.0-3 */

///
/// \brief get major version code from semantic version value
///
/// \param [in] v a semantic version code prepared by \c _mcci_arduino_verion_calc()
///
#define	_mcci_arduino_version_get_major(v)	\
	(((v) >> 24u) & 0xFFu)

///
/// \brief get minor version code from semantic version value
///
/// \param [in] v a semantic version code prepared by \c _mcci_arduino_verion_calc()
///
#define	_mcci_arduino_version_get_minor(v)	\
	(((v) >> 16u) & 0xFFu)

///
/// \brief get patch version code from semantic version value
///
/// \param [in] v a semantic version code prepared by \c _mcci_arduino_verion_calc()
///
#define	_mcci_arduino_version_get_patch(v)	\
	(((v) >> 8u) & 0xFFu)

///
/// \brief get pre-release version code from semantic version value
///
/// \param [in] v a semantic version code prepared by \c _mcci_arduino_verion_calc()
///
/// \note pre-release (local) 0 is *greater than* any pre-release value that's non-zero; use
///     \ref _mcci_arduino_version_calc() to compare relative versions.
///
#define	_mcci_arduino_version_get_local(v)	\
	((v) & 0xFFu)

/// \brief convert a semantic version to an integer.
#define _mcci_arduino_version_to_int(v)       \
        (((v) & 0xFFFFFF00u) | (((v) - 1) & 0xFFu))

/// \brief flag indicating that the version really is semantic
#define _mcci_arduino_version_is_semantic 1

///
/// \brief compare two semantic versions
///
/// \return \c true if \b a is less than \b b (as a semantic version).
///
#define _mcci_arduino_version_compare_lt(a, b)   \
        (_mcci_arduino_version_to_int(a) < _mcci_arduino_version_to_int(b))

///
/// \brief compare two semantic versions
///
/// \return \c true if \p a is greater than \p b (as a semantic version).
///
#define _mcci_arduino_version_compare_gt(a, b)   \
        (_mcci_arduino_version_to_int(a) >= _mcci_arduino_version_to_int(b))

///
/// \brief compare two semantic versions
///
/// \return \c true if \p a is greater than or equal to \p b (as a semantic version).
///
#define _mcci_arduino_version_compare_ge(a, b)   \
        (! _mcci_arduino_version_compare_lt(a, b))

///
/// \brief compare two semantic versions
///
/// \return \c true if \p a is less than or equal to \p b (as a semantic version).
///
#define _mcci_arduino_version_compare_le(a, b)   \
        (! _mcci_arduino_version_compare_gt(a, b))

#include "wiring.h"

/* sketch */

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus
extern void setup( void ) ;
extern void loop( void ) ;

void yield(void);
#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

// Include pins variant
#include "pins_arduino.h"

#endif // Arduino_h
