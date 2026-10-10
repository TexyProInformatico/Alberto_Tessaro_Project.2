#include "receiver.h"

extern "C" {
	#include "fake_receiver.h"
}

void receiver(){
        char message [MAX_CAN_MESSAGE_SIZE];
        while (!stop_rec){                              //so if stop_rec = true then rec must be stopped
                int msg_len = can_receive(message);     //i need the str lenght so yeah
                if (msg_len == -1) {                    //fake_reciever.h specifies it gives -1 if error
                        rec_fin = true;
                        main_con.notify_one();
                        break;
                }
                struct ReceivedMsg rec;                 //rec stands for received or more like rec as recording or recorded
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
