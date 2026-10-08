#include <stdio.h>
#include <iostream>
#include <string>	//need to get input from CAN
#include <cstdint>	//need to convert from string to exadecimal ecc...
#include <cctype>	//need to controll if inputs are exadecimal

extern "C"{
	#include "fake_receiver.h"
}

struct CanMessage{
	uint16_t id;
	uint8_t payload[8];
	int pay_len;
};

int main(void){

	//initials variables
	char message [MAX_CAN_MESSAGE_SIZE];
	open_can("../candump.log");		//i guess i messed up so i had to add ../
	int msg_len = can_receive(message);	//i need the str lenght so yeah

	std::string rec(message, msg_len);	//rec stands for received or more like rec as recording or recorded
	size_t divisor = rec.find('#');		//need to separate id and payload
	std::string str_id = rec.substr(0, divisor);
	std::string str_payload = rec.substr(divisor + 1);

	//control of the input
	if (str_id.length() != 3){		//id must be 3 digits
		std::cout << "Id size is not correct" << std::endl;
		return 1;
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
	msg.payload[8];
	msg.pay_len = str_payload.length() / 2;	//exadecimal so if i have lenght = n then the number of byte used is n/2

	for(int i = 0; i < msg.pay_len; i += 1){	//assing an array with the numbers in exadecimal base, why an array? idk wanted to track numbers
		std::string byte_of_payload = str_payload.substr(i * 2, 2);
		msg.payload[i] = std::stoul(byte_of_payload, nullptr, 16);
	}
	std::cout << msg.id << static_cast<int>(msg.payload[0]) << std::endl;
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

