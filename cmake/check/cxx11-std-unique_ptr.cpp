#include <memory>

int main() {
	std::unique_ptr<char> ptr(new char);
	return !ptr;
}
