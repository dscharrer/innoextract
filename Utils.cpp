
#include "Utils.h"

#include <algorithm>

using std::find;
using std::string;

string safestring(const char * data, size_t maxLength) {
	return string(data, std::find(data, data + maxLength, '\0'));
}
