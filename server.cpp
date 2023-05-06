#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <atomic>

namespace asio = boost::asio;
using boost::asio::ip::tcp;

#define IMG_SIZE 1920 * 1080 * 3
#define PACKAGE_SIZE 5000

const char* jpg_start = "\xff\xd8";
const size_t jpg_start_size = strlen(jpg_start);
const char* jpg_end = "\xff\xd9";
const size_t jpg_end_size = strlen(jpg_end);

char img[IMG_SIZE];
size_t size;
std::atomic<bool> img_is_ready;

ptrdiff_t find(const char* data, const size_t data_size, const char* target, const size_t target_size) {
	for (size_t i = 0; i < data_size - target_size + 1; i++) {
		for (size_t j = 0; j < target_size; j++) {
			if (data[i + j] != target[j]) goto end;
		}
		return i;
	end: continue;
	}
	return -1;
}

void handle_reciver(tcp::socket client);
void handle_sender(tcp::socket client);

int main() {
	img_is_ready.exchange(false);
	setlocale(LC_ALL, "russian");
	boost::system::error_code ec;
	asio::io_context context;
	tcp::endpoint address(tcp::v4(), 5000);
	tcp::acceptor acceptor(context, address);

	for (;;) {
		tcp::socket client = acceptor.accept(ec);
		if (ec) {
			std::cout << ec.message() << std::endl;
			continue;
		}
		uint8_t who;
		client.read_some(asio::buffer(&who, 1), ec);
		if (ec) {
			ec.clear();
			continue;
		}
		if (who == 1) {
			std::thread(handle_sender, std::move(client)).detach();
			continue;
		}
		if (who == 2) {
			std::thread(handle_reciver, std::move(client)).detach();
		}
	}
}

void handle_reciver(tcp::socket client) {
	boost::system::error_code ec;

	for (;;) {
		if (!img_is_ready.load()) continue;
		for (size_t i = 0; i < size; i += PACKAGE_SIZE) {
			size_t cur_package_size = size - i >= PACKAGE_SIZE ? PACKAGE_SIZE : size % PACKAGE_SIZE;
			client.send(asio::buffer(img+i, cur_package_size), NULL, ec);
			if (ec) {
				return;
			}
		}
		img_is_ready.exchange(false);
	}
}

void handle_sender(tcp::socket client) {
	boost::system::error_code ec;

	client.set_option(boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>{ 200 });

	char* buffer = new char[IMG_SIZE];

	size_t i = 0;
	size_t start;

	for (;;) {
		if (img_is_ready.load()) continue;
		size_t cur_size = client.read_some(asio::buffer(buffer+i, PACKAGE_SIZE), ec);
		if (ec) {
			break;
		}
		if (i == 0) {
			start = find(buffer, cur_size, jpg_start, jpg_start_size);
			if (start != -1) {
				i += cur_size;
			}
			continue;
		}
		size_t end = find(buffer + i, cur_size, jpg_end, jpg_end_size);
		if (end != -1) {
			end += i;
			i += cur_size;
			size = end - start + 2;
			memcpy(img, buffer+start, size);
			img_is_ready.exchange(true);
			memcpy(buffer, buffer + end + 2, i - end - 2);
			i -= end + 2;
			start = find(buffer, i, jpg_start, jpg_start_size);
			if (start == -1) i = 0;
			continue;
		}
		i += cur_size;
	}
	delete[] buffer;
}