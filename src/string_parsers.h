#ifndef STRING_PARSERS_H
#define STRING_PARSERS_H

#pragma once
#include <string>
#include <vector>

namespace fallout {

typedef int(StringParserCallback)(char* string, int* valuePtr);

int strParseInt(char** stringPtr, int* valuePtr);
int strParseStrFromList(char** stringPtr, int* valuePtr, const char** list, int count);
int strParseStrFromFunc(char** stringPtr, int* valuePtr, StringParserCallback* callback);
int strParseIntWithKey(char** stringPtr, const char* key, int* valuePtr, const char* delimeter);
int strParseKeyValue(char** stringPtr, char* key, int* valuePtr, const char* delimeter);

// Splits a string by a delimiter, returning a vector of trimmed tokens.
// Empty tokens are preserved (e.g., "a,,b" yields ["a", "", "b"]).
std::vector<std::string> splitString(const std::string& str, char delimiter = ',');

} // namespace fallout

#endif /* STRING_PARSERS_H */
