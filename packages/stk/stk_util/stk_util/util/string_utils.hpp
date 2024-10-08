// Copyright 2002 - 2008, 2010, 2011 National Technology Engineering
// Solutions of Sandia, LLC (NTESS). Under the terms of Contract
// DE-NA0003525 with NTESS, the U.S. Government retains certain rights
// in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
// 
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
// 
//     * Neither the name of NTESS nor the names of its contributors
//       may be used to endorse or promote products derived from this
//       software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 

#ifndef stk_util_string_utils_hpp
#define stk_util_string_utils_hpp

#include <string>  // for string
#include <vector>  // for vector

namespace stk {

std::string angle_it(const std::string &s);
std::string bracket_it(const std::string &s);
std::string dash_it(const std::string &s);
std::string rm_dashes(const std::string &s);

std::string get_substring_before_comma(const std::string& s);
std::string get_substring_after_comma(const std::string& s);

std::string tailname(const std::string& filename);
std::string basename(const std::string& filename);

std::vector<std::string> make_vector_of_strings(const std::string& inputString, char separator, int maxStringLength);

bool string_starts_with(const std::string & queryString, const std::string & prefix);

std::string ltrim_string(std::string s);
std::string rtrim_string(std::string s);
std::string trim_string(std::string s);
std::vector<std::string> split_string(const std::string & input, const char separator);
std::vector<std::string> split_csv_string(const std::string & input);

} // namespace stk

#endif /* stk_util_string_utils_hpp */

