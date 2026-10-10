#include <stdio.h>
#include <iostream>
#include <string>	//need to get input from CAN
#include <cstdint>	//need to convert from string to exadecimal ecc...
#include <cctype>	//need to controll if inputs are exadecimal
#include <queue>	//atp gonna add bunch of libraries
#include <mutex>	//ahaha i LOVE libraries
#include <thread>	//i want threads
#include <atomic>	//i... am... athomic... (yeah in v0.5 there was a h)
#include <condition_variable> //need comunications from threads
#include <fstream>	//use for logging and csv file
#include <chrono>	//need time
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <cstdint>
#include "receiver.h" 	//need to create a different file for the receiver thread

extern "C"{
	#include "fake_receiver.h"
}
//cration of the struct which will hold all the Can informations
struct CanMessage{
	uint16_t id;
	uint8_t payload[8];
	int pay_len;
};

//creation of the FSM
enum class State{
	Idle,
	Run,
	Before					//extra state i added bc the machine still has to start its process
};

/*struct ReceivedMsg {				//now in receiver.h
	std::string data;
	std::chrono::system_clock::time_point timestamp;
	std::chrono::steady_clock::time_point elapsted_timestamp; 	//yeah i'm not gonna abbreviate that otherwise i'll forget what it means
};
*/

//creation of the struct containing all the time stats of each id
struct Stats{
	unsigned int num_of_msg = 0;
	double tot_time_ms = 0.0;
	unsigned int num_of_intervals = 0;
	std::chrono::steady_clock::time_point last_timestamp;
};

//creation of global queue and variables
std::queue<ReceivedMsg> msgQueue;
std::mutex mutexQueue;
std::atomic<bool> stop_rec = false;
std::atomic<bool> rec_fin = false;
std::condition_variable main_con;
std::unordered_map<uint16_t, Stats> statistics;

//thread receiver
/*
void receiver(){
	char message [MAX_CAN_MESSAGE_SIZE];
	while (!stop_rec){				//so if stop_rec = true then rec must be stopped
		int msg_len = can_receive(message);	//i need the str lenght so yeah
		if (msg_len == -1) {			//fake_reciever.h specifies it gives -1 if error
			rec_fin = true;
			main_con.notify_one();
			break;
		}
		struct ReceivedMsg rec;			//rec stands for received or more like rec as recording or recorded
		rec.data = std::string(message, msg_len);
		rec.timestamp = std::chrono::system_clock::now();
		rec.elapsted_timestamp = std::chrono::steady_clock::now();
		{
		std::lock_guard<std::mutex> lock(mutexQueue);
		msgQueue.push(rec);
		}
		main_con.notify_one();
	}
}
*/

//save statistics
void saveStatistics (const std::string& filename) {
	std::ofstream csv(filename);
	if (!csv.is_open()) {
		std::cerr << "Error idk why but csv is not open" << std::endl;
		return;
	}
	csv << "ID,number_of_messages,mean_time\n";
	for (const auto& [id, stat] : statistics) {
		double mean = 0.0;
		if (stat.num_of_intervals > 0) {
			mean = stat.tot_time_ms / stat.num_of_intervals;
		}
		csv << std::hex << id << std::dec << ","
		    << stat.num_of_msg << "," << mean << '\n';
	}
}

int main(void){

	//preparations
	State state = State::Before;
	open_can("../candump.log");			//i guess i messed up so i had to add ../
	std::thread recThread(receiver);
	std::ofstream logFile;
	int ses_num = 0;				//aha ses

	//body of main
	while (true) {
		struct ReceivedMsg r_msg;
		bool is_msg_full = false;
		{					//this is in a block because of mutex that i don't want that variable in future things
			std::unique_lock<std::mutex> lock(mutexQueue);
			main_con.wait(lock, [] {
				return !msgQueue.empty() || rec_fin;
			});
			if(msgQueue.empty() && rec_fin){
				break;
			}
			r_msg = msgQueue.front();	//attaboy we have the massage
			msgQueue.pop();		//and we pop to get the next msg
			is_msg_full = true;
		}

		if (!is_msg_full) {
			continue;
		}

		//initialazing data of message
		std::string rec = r_msg.data;		//and we are soo back
		size_t divisor = rec.find('#');		//need to separate id and payload
		if (divisor == std::string::npos) {
			std::cout << "messaggio strano boh manca #" << std::endl;
			continue;
		}
		std::string str_id = rec.substr(0, divisor);
		std::string str_payload = rec.substr(divisor + 1);

		//initialazing timestamp of message
		auto time = std::chrono::system_clock::to_time_t(r_msg.timestamp);
		auto unix_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			       r_msg.timestamp.time_since_epoch()).count();
		std::ostringstream timestamp;
		timestamp << unix_ms / 1000.0;

		//initialazing elapsed timesss
		auto rec_time = r_msg.elapsted_timestamp;

		//control of the input
		if (str_id.empty() || str_id.length() > 3){		//id must be 3 digits
			std::cout << "Id size is not correct" << std::endl;
			continue;
		}
		for (char c : str_id){			//id must be written in exadecimal
			if(!std::isxdigit(c)){
				std::cout << "non exadecimal id" << std::endl;
				return 1;
			}
		}

		if (str_payload.length() % 2 != 0){	//payload is random lenght but its always even
			std::cout << "Payload isn't even" << std::endl;
			return 1;			//returns 1 because program ran in a error
		}
		if (str_payload.length() > 16){ 	//payload can't be longer than 8 byte
			std::cout << "payload exceed memory" << std::endl;
			return 1;			//same as 4 rows up
		}
		for (char c : str_payload){		//controls if the input from can is in exadecimal
			if(!std::isxdigit(c)){
				std::cout << "non exadeciaml payload" << std::endl;
				return 1;
			}
		}

		//conversion and parsing
		struct CanMessage msg;
		msg.id = std::stoul(str_id, nullptr, 16);
		msg.pay_len = str_payload.length() / 2;	//exadecimal so if i have lenght = n then the number of byte used is n/2

		for(int i = 0; i < msg.pay_len; i += 1){	//assing an array with the numbers in exadecimal base, why an array? idk wanted to track numbers
			std::string byte_of_payload = str_payload.substr(i * 2, 2);
			msg.payload[i] = std::stoul(byte_of_payload, nullptr, 16);
		}

		//calc of statistics
		Stats& stat = statistics[msg.id];
		if (stat.num_of_msg > 0) {
			double interval = std::chrono::duration<double, std::milli>(
					  rec_time - stat.last_timestamp).count();
			stat.tot_time_ms += interval;
			stat.num_of_intervals += 1;
		}
		stat.last_timestamp = rec_time;
		stat.num_of_msg += 1;

		//FSM
		if (msg.id == 0x0A0 && msg.pay_len == 2) {
			bool isStart = (msg.payload[0] == 0x66 && msg.payload[1] == 0x01) ||
	   	   		       (msg.payload[0] == 0xFF && msg.payload[1] == 0x01);
			bool isStop = msg.payload[0] == 0x66 && msg.payload[1] == 0xFF;
			if (isStart && state != State::Run) {	//We start to run after a stop or if its the first run of the cycle (before state)
				state = State::Run;
				ses_num += 1;
				auto now = std::chrono::system_clock::now();
				auto now_time = std::chrono::system_clock::to_time_t(now);
				std::ostringstream filename;
				filename << "tel_session_"
      				         << std::put_time(std::localtime(&now_time), "%Y%m%d_%H%M%S")
         				 << "_" << ses_num << ".log";

				logFile.open(filename.str());
			}
			else if(isStop && state == State::Run) {	//wants to stop only if i run
				state = State::Idle;
				if (logFile.is_open()) {
					logFile.close();
				}
				saveStatistics("statistics.csv");
			}
		}

		//logging
		if (state == State::Run && logFile.is_open()) {
			logFile << timestamp.str() << " | " << rec << '\n';
		}
	}

	recThread.join();
	close_can();

	return 0;
}


	//test of code for optimal output

	/*						//used to test if output was correct in the early version of the code
	std::cout << "l'output è ";
	for(int i = 0; i < msg_len; i += 1){
		std::cout << message[i];
	}
	*/

	/*						//used to test the correct division from id and payload
	std::cout << "id è " << id << std::endl;
	std::cout << "payload è " << payload << std::endl;
	*/

	/*						//used in early version to parse the input before struct CanMessage
	uint16_t id = std::stoul(str_id, nullptr, 16);
	uint8_t payload[8];
	int pay_len = str_payload.length() / 2;
	*/

	//std::cout << std::hex << msg.id << static_cast<int>(msg.payload[0]) << std::endl;     control of the ouput

	/*						//early version of the reciever in main, take if needed
	int msg_len = can_receive(message);
	if (msg_len == -1) {
		break;
	}
	std::string rec(message, msg_len);
	*/

	/*						//eary fsm
	if (state == State::Idle || state == State::Before){
		if(msg.id == 0x0A0 && msg.pay_len == 2){	//check the id and the payload for start of the fsm
			if((msg.payload[0] == 0x66 && msg.payload[1] == 0x01) ||
	   	   	   (msg.payload[0] == 0xFF && msg.payload[1] == 0x01)){
				state = State::Run;
				std::cout << "start" << std::endl;
			}
		}
	}
	if (state == State::Run || state == State::Before){
		if(msg.id == 0x0A0 && msg.pay_len == 2){
			if(msg.payload[0] == 0x66 && msg.payload[1] == 0xFF){
				state = State::Idle;
				std::cout << "idle" << std::endl;
			}
		}
	}
	*/

	/*	//stampa di id e payload
	std::cout << "ID: " << std::hex << msg.id << " | Payload: ";

	for (int i = 0; i < msg.pay_len; i+= 1) {
		std::cout << std::hex << static_cast<int>(msg.payload[i]) << " ";
	}
	std::cout << std::endl;
	*/

	/*
	timestamp << std::put_time(std::localtime(&time),  "%d-%m-%Y %H:%M:%S")
			  << "." << std::setfill('0') << std::setw(3) << ms;
	*/


