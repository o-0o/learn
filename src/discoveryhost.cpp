#include "../include/discoveryhost.h"

void init_broadcast(Host &host);
// void checkAlive(Host &host);
// void run_callbacks(Host &host,int type,void *arg,void *msg) ;
// void register_callback(Host &host,Callback callback);
void capture_broadcast_address(int inet_sock,std::vector<struct in_addr> &addrs) {
    struct ifconf ifc;
    struct ifreq *ifr;
    char buf[1024];
    int num_ifs;

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(inet_sock, SIOCGIFCONF, &ifc) == -1) {
        perror("ioctl(SIOCGIFCONF)");
        exit(1);
    }
    ifr = ifc.ifc_req;
    num_ifs = ifc.ifc_len / sizeof(struct ifreq);
    for (int i = 0; i < num_ifs; ++i) {
        if(ifr[i].ifr_name[0]=='l'){
            continue;
        }else{
            if (ioctl(inet_sock, SIOCGIFBRDADDR, &ifr[i]) == 0) {
                addrs.push_back(((struct sockaddr_in *)&ifr[i].ifr_addr)->sin_addr);
                //print brocastaddress
                // std::cout<< inet_ntoa(((struct sockaddr_in *)&ifr[i].ifr_addr)->sin_addr)<<std::endl;
            }
        }

    }
}

// void print_device_list(Host &host,std::vector<struct sockaddr_in> &addrs){
void print_device_list(Host &host){
    for(int i=0;i<host.devs.size();i++){
        if(host.devs[i].addresses.size()==0)
            continue;
        std::cout<<host.devs[i].msg.host_id<<std::endl;
        for(int j=0;j<host.devs[i].addresses.size();j++){
            // addrs.push_back(host.devs[i].addresses[j].address);
            std::cout<<"     ip:"<<inet_ntoa(host.devs[i].addresses[j].address.sin_addr)<<" image_port:"<<host.devs[i].msg.image_port<<" control_port"<<host.devs[i].msg.control_port<<std::endl;
        }
    }
}

void discovery_initHost(Host &host,in_port_t port){
    host.port=port;
    host.running=false;
    host.interval=1;
    host.recv_timeout=1;

}

void discovery_device_list(Host &host,struct sockaddr_in &device_addr,Message &msg){
    int device_id_find=0;
    int device_id_index=0;
    int address_id_index=0;
    for(int i=0;i<host.devs.size();i++){
        if(host.devs[i].msg.host_id==msg.host_id){
            device_id_index=i;
            device_id_find=1;
            break;
        }
    }
    if(!device_id_find){
        Dev dev;
        dev.msg.host_id=msg.host_id;
        dev.msg.image_port=msg.image_port;
        dev.msg.control_port=msg.control_port;
        host.devs.push_back(dev);
        // run_callbacks(host, NEW_DEVICE,&addr.address,&dev.msg);


        Address addr;
        addr.address=device_addr;
        addr.scantime=time(NULL);
        host.devs[host.devs.size()-1].addresses.push_back(addr);
        // run_callbacks(host, NEW_ADDRESS,&addr.address,&dev.msg);
        host.devs[host.devs.size()-1].msg.ips.push_back(inet_ntoa(device_addr.sin_addr));
    }else{
        int address_id_find=0;
        for(int j=0;j<host.devs[device_id_index].addresses.size();j++){
            if(host.devs[device_id_index].addresses[j].address.sin_addr.s_addr==device_addr.sin_addr.s_addr){
                address_id_index=j;
                address_id_find=1;
                break;
            }
        }
        if(!address_id_find){
            Address addr;
            addr.address=device_addr;
            addr.scantime=time(NULL);
            host.devs[device_id_index].addresses.push_back(addr);
            // run_callbacks(&host, NEW_ADDRESS,&addr.address,&host.devs[device_id_index].msg);
            host.devs[device_id_index].msg.ips.push_back(inet_ntoa(device_addr.sin_addr));
        }else{
            host.devs[device_id_index].addresses[address_id_index].scantime=time(NULL);
        }
    }
}

time_t run_time=time(NULL);
bool host_broadcast(Host &host){
    init_broadcast(host);

    struct sockaddr_in device_addr;
    size_t addr_len = sizeof(device_addr);
    Message msg;

    std::vector<struct in_addr> addrs;
    capture_broadcast_address(host.sock,addrs);

    for(int i=0;i<addrs.size();i++){
        host.broadcast_addr.sin_addr.s_addr = addrs[0].s_addr;
        // host.broadcast_addr.sin_addr.s_addr = inet_addr("172.28.255.255");

        ssize_t size = sendto(host.sock, &msg, sizeof(msg), 0, (struct sockaddr*)&host.broadcast_addr, addr_len);

        while(1){
            if(!host.running)
                if(time(NULL)-run_time>1)
                    break;
            ssize_t count = recvfrom(host.sock, &msg, sizeof(msg), 0, (struct sockaddr*)&device_addr, (socklen_t*)&addr_len);
            if(count>=0){
                msg.host_id=ntohl(msg.host_id);
                msg.control_port=ntohs(msg.control_port);
                msg.image_port=ntohs(msg.image_port);
                discovery_device_list(host,device_addr,msg);
            }
            // checkAlive(host);
        }
    }
    close(host.sock);
    if(host.devs.size()>0)
        return true;
    else
        return false;
}

void init_broadcast(Host &host){
    host.sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (host.sock < 0) {
        perror("host sock error");
    }

    struct timeval tv;
    tv.tv_sec=host.recv_timeout;
    tv.tv_usec=0;
    int opt=1;
    setsockopt(host.sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(host.sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
    setsockopt(host.sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    memset((void*)&host.broadcast_addr, 0, sizeof(host.broadcast_addr));
    host.broadcast_addr.sin_family = AF_INET;
    host.broadcast_addr.sin_port = htons(host.port);
        
}


// void checkAlive(Host &host) {
//     Message msg;
//     for(int i=0;i<host.devs.size();i++){
//         for(int j=0;j<host.devs[i].addresses.size();j++){
//             if(host.devs[i].addresses[j].scantime+ALIVE_TIME<time(NULL)){
//                 run_callbacks(host,ADDRESS_NOT_ALIVE,&host.devs[i].addresses[j].address,&host.devs[i].msg);
//                 host.devs[i].addresses.erase(host.devs[i].addresses.begin()+j);
//             }
//         }
//         if(host.devs[i].addresses.size()==0){
//             run_callbacks(host,DEVICE_NOT_ALIVE,NULL,&host.devs[i].msg);
//             host.devs.erase(host.devs.begin()+i);
//         }
//     }
// }

// void run_callbacks(Host &host,int type,void *arg,void *msg) {
//     for (uint16_t i = 0; i < host.callbacks.size(); i++) {
//         Callback callback = host.callbacks[i];
//         callback(arg,type,msg);
//     }
// }


// void register_callback(Host &host,Callback callback){
//     host.callbacks.push_back(callback);
// }


