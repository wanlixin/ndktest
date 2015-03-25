#include <asio.hpp>
#include <android/log.h>

using namespace asio;
typedef local::stream_protocol sp;
typedef local::datagram_protocol dp;

void log(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	__android_log_vprint(ANDROID_LOG_ERROR, "testasiodgram", fmt, args);
	va_end(args);
}

class session: public std::enable_shared_from_this<session> {
public:
	session(sp::socket socket) :
			socket_(std::move(socket)) {
	}

	void start() {
		auto self(shared_from_this());
		socket_.async_read_some(asio::buffer(data_, max_length),
				[this, self](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						data_[length] = 0;
						log("%s\n", data_);
					}
				});
	}

private:
	sp::socket socket_;
	enum {
		max_length = 1024
	};
	char data_[max_length];
};

class stream_server {
public:
	stream_server(asio::io_service& io_service, const char* name) :
			acceptor_(io_service,
					sp::endpoint(std::string(name, strlen(&name[1]) + 1))), socket_(
					io_service) {
		do_accept();
	}

private:
	void do_accept() {
		acceptor_.async_accept(socket_, [this](std::error_code ec)
		{
			if (!ec)
			{
				std::make_shared<session>(std::move(socket_))->start();
			}

			do_accept();
		});
	}

	sp::acceptor acceptor_;
	sp::socket socket_;
};

class datagram_server {
public:
	datagram_server(asio::io_service& io_service, const char* name) :
			socket_(io_service,
					dp::endpoint(std::string(name, strlen(&name[1]) + 1))) {

		do_receive();
	}

	void do_receive() {
		socket_.async_receive(asio::buffer(data_, max_length),

		[this](std::error_code ec, std::size_t length)
		{
			if (!ec) {
				data_[length] = 0;
				log("%s\n", data_);
				do_receive();
			}
		});
	}

private:
	dp::socket socket_;
	enum {
		max_length = 1024
	};
	char data_[max_length];
};

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("usages:\ttestasiodgram -server\n\ttestasiodgram message\n");
		return 0;
	}

	try {
		asio::io_service io_service;

		if ((strcmp(argv[1], "-server") == 0)) {
			stream_server ss(io_service, "\0teststream");
			datagram_server ds(io_service, "\0testdgram");

			io_service.run();
		} else {

			char text[1024];

			try {
				asio::io_service io_service;
				const char* name = "\0teststream";
				sprintf(text, "stream:%s", argv[1]);

				sp::socket socket(io_service);
				socket.connect(
						sp::endpoint(std::string(name, strlen(&name[1]) + 1)));
				socket.send(asio::buffer(text, strlen(text)));

			} catch (std::exception& e) {
				log("%s\n", e.what());
			}
			try {
				const char* name2 = "\0testdgram";
				sprintf(text, "datagram:%s", argv[1]);
				dp::socket(io_service, dp()).send_to(
						asio::buffer(text, strlen(text)),
						dp::endpoint(
								std::string(name2, strlen(&name2[1]) + 1)));
			} catch (std::exception& e) {
				log("%s\n", e.what());
			}
		}
	} catch (std::exception& e) {
		log("%s\n", e.what());
	}

	return 0;
}
