/*
 * Copyright (C) 2012-2019 Daniel Scharrer
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the author(s) be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/*!
 * \file
 *
 * Functions to evaluation Inno Setup boolean expressions.
 */
#ifndef INNOEXTRACT_SETUP_EXPRESSION_HPP
#define INNOEXTRACT_SETUP_EXPRESSION_HPP

#include <string>

namespace setup {

/*
 * Determine if the given expression is satisfied with (only) the given test variable set to true
 */
bool expression_match(const std::string & test, const std::string & expression);

bool is_simple_expression(const std::string & expression);

} // namespace setup

#endif // INNOEXTRACT_SETUP_EXPRESSION_HPP
