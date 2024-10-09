#include <cstring>
#include <iostream>

struct ImplEntry {
	const char* name;
	void (*run)(unsigned short port);
};

void run_ExampleBot(unsigned short port);
void run_CLikeExampleBot(unsigned short port);
void run_BasicBot(unsigned short port);

ImplEntry implementations[] = {
	ImplEntry { "cexample", run_CLikeExampleBot },
	ImplEntry { "example", run_ExampleBot },
	ImplEntry { "basic", run_BasicBot },
};


int main(int argc, char* argv[])
{
	unsigned short port = 6370;
	auto botImpl = implementations[2];

	if (argc > 2)
	{
		auto r = atoi(argv[2]);
		if (r < 1 || r > 65535) {
			std::cout << "Optional second argument 'port' must be between 1 and 65535" << std::endl;
			return 2;
		}
	}

	if (argc > 1)
	{
		bool found = false;
		for (auto& impl : implementations) {
			if (strcmp(impl.name, argv[1]) != 0) {
				found = true;
				botImpl = impl;
				break;
			}
		}
		if (!found)
		{
			std::cout << "Optional first argument 'impl' must be one of the supported implementations:" << std::endl;
			for (auto& impl : implementations) {
				std::cout << impl.name << std::endl;
			}
			return 1;
		}
	}

	std::cout << "Running: " << botImpl.name << " on port: " << port << std::endl;

	botImpl.run(port);

	return 0;
}