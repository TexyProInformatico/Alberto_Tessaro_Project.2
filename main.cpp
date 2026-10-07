#include <stdio.h>
#include <iostream>
#include <string>	//need to get input from CAN
#include <cstdint>	//need to convert from string to exadecimal ecc...

extern "C"{
	#include "fake_receiver.h"
}

int main(void){

	char message [MAX_CAN_MESSAGE_SIZE];
	open_can("../candump.log");		//i guess i messed up so i had to add ../
	int msg_len = can_receive(message);	//i need the str lenght so yeah

	std::string rec(message, msg_len);	//rec stands for received or more like rec as recording or recorded
	size_t divisor = rec.find('#');		//need to separate id and payload
	std::string str_id = rec.substr(0, divisor);
	std::string str_payload = rec.substr(divisor + 1);

	if (str_payload.length() % 2 != 0){	//payload is random lenght but its always even
		std::cout << "Error, invalid input or smth like that" << std::endl;
		return 1;			//returns 1 because program ran in a error
	}
	if (str_payload.length() > 16){ 	//payload can't be longer than 8 byte
		std::cout << "payload exceed memory" << std::endl;
		return 1;			//same as 4 rows up
	}

	unsigned long id_n = std::stoul(str_id, nullptr, 16);
	unsigned long payload_n = std::stoul(str_payload, nullptr, 8);
	int pay_len = str_payload.length() / 2;	//exadecimal so if i have lenght = n then the number of byte used is n/2

	uint16_t id = std::stoul(str_id, nullptr, 16);
	uint8_t payload[8];

	for(int i = 0; i < pay_len; i += 1){	//assing an array with the numbers in exadecimal base, why an array? because i though it was hte only way to keep tracking for a couple of numbers
		std::string byte_of_payload = str_payload.substr(i * 2, 2);
		payload[i] = std::stoul(byte_of_payload, nullptr, 16);
	}
	std::cout << id << static_cast<int>(payload[0]) << std::endl;
	close_can();

	return 0;
}


	//test of code for optimal output

	/*
	std::cout << "l'output è ";		//used to test if output was correct in the early version of the code
	for(int i = 0; i < msg_len; i += 1){
		std::cout << message[i];
	}
	*/

	/*
	std::cout << "id è " << id << std::endl;	//used to test the correct division from id and payload
	std::cout << "payload è " << payload << std::endl;
	*/
