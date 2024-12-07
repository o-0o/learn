#include "../include/discoveryhost.h"

void notification(void *arg,int type,void *msg){
    struct sockaddr_in *addr= (struct sockaddr_in *)arg;
    Message *message=(Message *)msg;
    switch(type){
        case NEW_DEVICE:std::cout<<"new device"<<std::endl;break;
        case NEW_ADDRESS:std::cout<<"new address"<<std::endl;
            if(addr)
                std::cout<<"ip:"<<inet_ntoa(addr->sin_addr)<<" id:"<<message->host_id<<" image_port:"<<message->image_port<<" control_port:"<<message->control_port<<std::endl;
            break;
        case ADDRESS_NOT_ALIVE:std::cout<<"address not alive"<<std::endl;
            if(addr)
                std::cout<<"ip:"<<inet_ntoa(addr->sin_addr)<<" id:"<<message->host_id<<" image_port:"<<message->image_port<<" control_port:"<<message->control_port<<std::endl;
            break;
        case DEVICE_NOT_ALIVE:std::cout<<"device not alive"<<std::endl;break;
        default:std::cout<<"unknown notification"<<std::endl; 
    }
}

void scan_once(Host &host){
    bool ret=host_broadcast(host);
    if(ret){
        for(int i=0;i<host.devs.size();i++){
                std::cout<<" id:"<<host.devs[i].msg.host_id<<std::endl;
            for(int j=0;j<host.devs[i].msg.ips.size();j++){
                std::cout<<"     ip:"<<host.devs[i].msg.ips[j];
                std::cout<<"  image_port:"<<host.devs[i].msg.image_port<<"  control_port:"<<host.devs[i].msg.control_port<<std::endl;
            }
        }
    }else
        std::cout<<"don't search device."<<std::endl;
}

// void scan_loop(Host &host){
//     register_callback(host,notification);
//     host.running=true;
//     host_broadcast(host);
// }

#include <iostream>
int main(int argc, char const *argv[])
{
    Host host;
    discovery_initHost(host,12345);
    scan_once(host);
    // scan_loop(host);
    
    return 0;
}
