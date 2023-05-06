#include <iostream>
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>

namespace asio = boost::asio;
using boost::asio::ip::tcp;
using boost::asio::ip::address;

#define IMG_SIZE 1920 * 1080 * 3
#define PACKAGE_SIZE 5000

const char* jpg_start = "\xff\xd8";
const size_t jpg_start_size = strlen(jpg_start);
const char* jpg_end = "\xff\xd9";
const size_t jpg_end_size = strlen(jpg_end);

const uint8_t on_connect_message = 2;

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

void show_stream();

int main() {
    img_is_ready.exchange(false);
	setlocale(LC_ALL, "russian");
	boost::system::error_code ec;
	asio::io_context context;
	tcp::socket client(context);
	tcp::endpoint remote_address(address::from_string("192.168.0.105"), 5000);
	client.connect(remote_address, ec);
	if (ec) {
		return ec.value();
	}

	client.write_some(asio::buffer(&on_connect_message, 1), ec);
	if (ec) {
		return ec.value();
	}

	char* buffer = new char[IMG_SIZE];

	size_t i = 0;
	size_t start;

	std::thread(show_stream).detach();

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
		size_t end = find(buffer+i, cur_size, jpg_end, jpg_end_size);
		if (end != -1) {
			end += i;
			i += cur_size;
			size = end - start + 2;
			memcpy(img, buffer + start, size);
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

void show_stream() {
	cv::namedWindow("Lansky gay");
	for (;;) {
		if (!img_is_ready.load()) continue;

		cv::Mat cvimg = cv::imdecode(cv::Mat(1, size, CV_8UC1, img), cv::IMREAD_COLOR);
		img_is_ready.exchange(false);
		if (cvimg.empty()) continue;
		cv::imshow("Lansky gay", cvimg);
		cv::waitKey(5);
	}
}