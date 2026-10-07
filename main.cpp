#include <stdio.h>
#include <iostream>
#include <string>

extern "C"{
	#include "fake_receiver.h"
}

int main(void){

	char message [MAX_CAN_MESSAGE_SIZE];
	open_can("../candump.log");	//i guess i messed up so i had to add ../
	int msg_len = can_receive(message);	//i need the str lenght so yeah

	std::string rec(message, msg_len);	//rec stands for received or more like rec as recording or recorded
	size_t divisor = rec.find('#');		//need to separate id and payload
	std::string id = rec.substr(0, divisor);
	std::string payload = rec.substr(divisor + 1);

	/*
	std::cout << "l'output è ";		//used to test if output was correct in the early version of the code
	for(int i = 0; i < msg_len; i += 1){
		std::cout << message[i];
	}
	*/

	std::cout << "id è " << id << std::endl;
	std::cout << "payload è " << payload << std::endl;
	std::cout << std::endl;
	close_can();

	return 0;
}
