/*
 * Copyright (c) 2025 @https://github.com/kdilayer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#pragma once

#include "config_app.h"

#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <chrono>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <vector>
#include <string>

#include <iostream>
#include <string>
#include <stdlib.h>

std::string formattedString(const char *format, ...);
std::string ReadFileContent(std::string filename);
bool file_exists (const std::string& name);
bool WriteFileContent(std::string filename, std::string &content, bool overwrite=false);
std::string timestamp_to_string(int ts_seconds, bool print_hours = false);
bool string_startswith(const char* haystack, size_t haystackSize, const char* needle, size_t needleSize);
bool string_startswith(const std::string haystack, const std::string needle);
bool string_endswith(const std::string& haystack, const std::string& needle);
std::string string_trim(const std::string& str, const std::string& whitespace);
void string_replaceall(std::string& source, const std::string& from, const std::string& to);
std::string string_tolower(const std::string& s);
bool string_contains_lower(const std::string& haystack, const std::string& needle);
bool string_contains(const std::string& haystack, const std::string& needle);
std::vector<std::string> string_split(const std::string& input, const std::string& delimiters);
bool string_is_number(const std::string& s);
std::string iso_8859_1_to_utf8(const std::string& input);

std::string escape_json(const std::string &s);
std::string getCountryCodeFromName(const std::string& name);
bool startsWithCountryCode(const std::string& str);
std::string getFirstTwoChars(const std::string& str);
std::string getCountryNameForCode(const std::string& code);
