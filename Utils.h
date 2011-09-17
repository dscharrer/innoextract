
#include <iostream>
#include <string>

template <class T>
inline std::istream & read(std::istream & ifs, T & data) {
	return ifs.read(reinterpret_cast<char *>(&data), sizeof(T));
}
/**
 * Load an std::string from a const char * that may not be null-terminated.
 */
std::string safestring(const char * data, size_t maxLength);

template <size_t N>
std::string safestring(const char (&data)[N]) {
	return safestring(data, N);
}

