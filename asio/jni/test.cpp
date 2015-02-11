#include <asio.hpp>

int main()
{
	asio::io_service _io_service;
	_io_service.run();
	return 0;
}