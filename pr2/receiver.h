#ifndef RECEIVER_H
#define RECEIVER_H

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>

struct ReceivedMsg {
	std::string data;
	std::chrono::system_clock::time_point timestamp;
	std::chrono::steady_clock::time_point elapsted_timestamp; 	//yeah i'm not gonna abbreviate that otherwise i'll forget what it means
};

extern std::queue<ReceivedMsg> msgQueue;
extern std::mutex mutexQueue;
extern std::atomic<bool> stop_rec;
extern std::atomic<bool> rec_fin;
extern std::condition_variable main_con;

void receiver();

#endif
