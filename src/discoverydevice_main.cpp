#include "../include/discoverydevice.h"


// void notification(int type){
//     switch(type){
//         case NEW_HOST:std::cout<<"new host"<<std::endl;break;
//         case NEW_ADDRESS:std::cout<<"new address"<<std::endl;break;
//         case ADDRESS_NOT_ALIVE:std::cout<<"address not alive"<<std::endl;break;
//         case HOST_NOT_ALIVE:std::cout<<"host not alive"<<std::endl;break;
//         default:std::cout<<"unknown notification"<<std::endl; 
//     }
// }

int main(int argc, char const *argv[])
{
    int ret = 0;
    Device device;

    Message msg={.device_id=123,
                 .image_port=8081,
                 .control_port=8080};

    discovery_initDevice(device,12345,msg);
    
    // register_callback(&device,notification);
    // discovery_start_broadcast(&device);


    return 0;
}