#include <asio.hpp>

using namespace asio;
typedef local::datagram_protocol dp;

class server {
public:
	server(asio::io_service& io_service, const std::string& name) :
			socket_(io_service, dp()) {

		std::error_code ec;
		socket_.bind(dp::endpoint(name), ec);
		if (ec) {
			printf("bind: ec:%d %s\n", ec.value(), ec.message().c_str());
			return;
		}

		do_receive();
	}

	void do_receive() {
		socket_.async_receive(asio::buffer(data_, max_length),

				[this](std::error_code ec, std::size_t bytes_recvd)
				{
					if (!ec && bytes_recvd > 0)
					{
						do_send(bytes_recvd);
					}
					else
					{
						if (ec) {
							printf("receive: ec:%d %s\n", ec.value(), ec.message().c_str());
						}
						if (bytes_recvd <= 0) {
							printf("receive: %d\n", bytes_recvd);
						}

						do_receive();
					}
				});
	}

	void do_send(std::size_t length) {
		data_[length] = 0;
		printf("received: %s\n", data_);
		do_receive();
	}

private:
	dp::socket socket_;
	enum {
		max_length = 1024
	};
	char data_[max_length];
};

class client {
public:
	client(asio::io_service& io_service, const std::string& name,
			const char* str) :
			socket_(io_service, dp()), sender_endpoint_(name) {
		socket_.async_send_to(asio::buffer(str, strlen(str)), sender_endpoint_,
				[this](std::error_code ec, std::size_t bytes_sent)
				{
					if (ec) {
						printf("sendto: ec:%d %s\n", ec.value(), ec.message().c_str());
					}
					if (bytes_sent <= 0) {
						printf("sendto: %d\n", bytes_sent);
					}
				});
	}

private:
	dp::socket socket_;
	dp::endpoint sender_endpoint_;
};

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("usages:\ttestasiodgram -server\n\ttestasiodgram message\n");
		return 0;
	}

	bool bServer = (strcmp(argv[1], "-server") == 0);

	io_service ios;

	const char* name = "\0testdgram";
	size_t namelen = strlen(&name[1]);

	std::string _name((const char*) name, namelen + 1);

	if (bServer) {
		server s(ios, _name);
		ios.run();
	} else {
		client c(ios, _name, argv[1]);
		ios.run();
	}

	return 0;
}
