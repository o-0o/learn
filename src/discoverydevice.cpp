#include "../include/discoverydevice.h"



void device_receiver(Device &device){
    Message msg;
    struct sockaddr_in recv_addr;
    socklen_t recv_addr_len=sizeof(struct sockaddr_in);
    msg.device_id=htonl(device.message.device_id);
    msg.image_port=htons(device.message.image_port);
    msg.control_port=htons(device.message.control_port);
    const char* str[] = {"|", "/", "-", "\\"};
    int count=0;
    bool show=true;
    while(device.running){
        Message ms;

        if(recvfrom(device.sock,&ms,sizeof(ms),0,(struct sockaddr *)&recv_addr,&recv_addr_len)<0){
            std::cout<<"\r"<<str[count]<<"runing.."<<std::flush;
            count=(count+1)%4; 
            show=true;
        }else{
            if(show){
                std::cout<<std::endl<<"id:"<<device.message.device_id<<std::endl;
                std::cout<<"     ip:"<<inet_ntoa(recv_addr.sin_addr)<<" image_port:"<<device.message.image_port<<" control_port:"<<device.message.control_port<<std::endl;
                show=false;
            }
            if(sendto(device.sock,&msg,sizeof(msg),0,(struct sockaddr *)&recv_addr,recv_addr_len)<0){
                std::cout<<"send failed"<<std::endl;
            }
        }
    }
}

void init_udp(Device &device){

    device.sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (device.sock < 0) {
        perror("device sock error");
    }

    struct timeval tv;
    tv.tv_sec=device.recv_timeout;
    tv.tv_usec=0;
    int opt=1;
    int ret;
    ret= setsockopt(device.sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    ret= setsockopt(device.sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
    ret= setsockopt(device.sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    memset((void*)&device.device_addr, 0, sizeof(device.device_addr));
    device.device_addr.sin_family = AF_INET;
    device.device_addr.sin_port = htons(device.port);
    device.device_addr.sin_addr.s_addr=htonl(INADDR_ANY);

    bind(device.sock,(struct sockaddr *)&device.device_addr,sizeof(device.device_addr));

    device.running=true;
    device_receiver(device);
}

void discovery_initDevice(Device &device,in_port_t port,Message &msg){
    device.port=port;
    device.running=false;
    device.interval=1;
    device.recv_timeout=1;
    device.message.device_id=msg.device_id;
    device.message.image_port=msg.image_port;
    device.message.control_port=msg.control_port;

    init_udp(device);
}


// void discovery_stop(Device *device) {
//     pthread_join(device->receiver_thread, NULL);
// }

// void checkAlive(Device *device) {
//     for(int i=0;i<device->devs.size();i++){
//         for(int j=0;j<device->devs[i].addresses.size();j++){
//             if(device->devs[i].addresses[j].scantime+ALIVE_TIME<time(NULL)){
//                 device->devs[i].addresses.erase(device->devs[i].addresses.begin()+j);
//                 run_callbacks(device,ADDRESS_NOT_ALIVE);
//             }
//         }
//         if(device->devs[i].addresses.size()==0){
//             device->devs.erase(device->devs.begin()+i);
//             run_callbacks(device,HOST_NOT_ALIVE);
//         }
//     }
// }
// void run_callbacks(Device *device,int type) {
//     for (uint16_t i = 0; i < device->callbacks.size(); i++) {
//         Callback callback = device->callbacks[i];
//         callback(type);
//     }
// }
// void *broadcast(void *arg) {
//     Device *device = (Device *)arg;
//     struct sockaddr_in device_addr;
//     size_t addr_len = sizeof(device_addr);
//     Message msg;

//     std::vector<struct in_addr> addrs;
//     capture_broadcast_address(device->sock,addrs);
//     memset((void*)&device->broadcast_addr, 0, addr_len);
//     device->broadcast_addr.sin_family = AF_INET;
//     device->broadcast_addr.sin_port = htons(device->port);
//     while (device->running)
//     {
//         ssize_t count = recvfrom(device->sock, &msg, sizeof(msg), 0, (struct sockaddr*)&device_addr, (socklen_t*)&addr_len);
//         for(int i=0;i<addrs.size();i++){
//             device->broadcast_addr.sin_addr.s_addr = inet_addr(inet_ntoa(addrs[i]));
//             sender(device);
//         }
//         if (count >= 0) {
//             msg.device_id=ntohl(msg.device_id);            
//             msg.image_port=ntohs(msg.image_port);
//             msg.control_port=ntohs(msg.control_port);
//             int dev_id_find=0;
//             int dev_id_index=0;
//             int address_id_index=0;
//             for(int i=0;i<device->devs.size();i++){
//                 if(device->devs[i].id==msg.device_id){
//                     dev_id_index=i;
//                     dev_id_find=1;
//                     break;
//                 }
//             }
//             if(!dev_id_find){
//                 Dev dev;
//                 dev.id=msg.device_id;
//                 device->devs.push_back(dev);
//                 run_callbacks(device, NEW_HOST);
//                 Address addr;
//                 addr.address=device_addr;
//                 addr.scantime=time(NULL);
//                 device->devs[dev_id_index].addresses.push_back(addr);
//                 run_callbacks(device, NEW_ADDRESS);
//                 std::cout<<device->message.device_id<<":"<<std::endl;
//                 for(int i=0;i<device->devs.size();i++)
//                     for(int j=0;j<device->devs[i].addresses.size();j++)
//                         std::cout<<"     ip:"<<inet_ntoa(device->devs[i].addresses[j].address.sin_addr)<<" port:"<<device->message.image_port<<" control_port:"<<device->message.control_port<<std::endl;
//                         // std::cout<<"     ip:"<<inet_ntoa(device->devs[i].addresses[j].address.sin_addr)<<" image_port:"<<device->message.image_port<<" control_port:"<<device->message.control_port<<std::endl;
//                 // device->running=0;
//                 // close(device->sock);
//                 // break;
//             }else{
//                 int address_id_find=0;
//                 for(int j=0;j<device->devs[dev_id_index].addresses.size();j++){
//                     if(device->devs[dev_id_index].addresses[j].address.sin_addr.s_addr==device_addr.sin_addr.s_addr){
//                         address_id_index=j;
//                         address_id_find=1;
//                         break;
//                     }
//                 }
//                 if(!address_id_find){
//                     Address addr;
//                     addr.address=device_addr;
//                     addr.scantime=time(NULL);
//                     device->devs[dev_id_index].addresses.push_back(addr);
//                     run_callbacks(device, NEW_ADDRESS);
//                     std::cout<<device->message.device_id<<":"<<std::endl;
//                     for(int i=0;i<device->devs.size();i++)
//                         for(int j=0;j<device->devs[i].addresses.size();j++){
//                             std::cout<<"     ip:"<<inet_ntoa(device->devs[i].addresses[j].address.sin_addr)<<" port:"<<ntohs(device->devs[i].addresses[j].address.sin_port)<<std::endl;
//                         }
//                 }else{
//                     device->devs[dev_id_index].addresses[address_id_index].scantime=time(NULL);
//                     // std::cout<<device->message.device_id<<":{("<<inet_ntoa(device->devs[dev_id_index].addresses[address_id_index].address.sin_addr)<<"):"<<device->devs[dev_id_index].addresses[address_id_index].scantime<<"}"<<std::endl;
//                 }
//             }
//         }else if(count==-1){
//             // std::cout<<"waiting for broadcast message timed out"<<std::endl;
//         }
//         checkAlive(device);
//     }
//     return NULL;
// }

// void register_callback(Device *device,Callback callback){
//     device->callbacks.push_back(callback);
// }