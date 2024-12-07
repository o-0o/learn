#include "../include/discoveryhost.h"

void cout_device(Host &host){
    std::cout<<" id:"<<host.devs[host.devs.size()-1].msg.host_id<<std::endl;
}
void cout_addr(Host &host){
    // for(int i=0;i<host.devs.size();i++){
    int i=host.devs.size()-1;
        for(int j=0;j<host.devs[i].addresses.size();j++){
            std::cout<<"     ip:"<<inet_ntoa(host.devs[i].addresses[j].address.sin_addr);
            std::cout<<"  image_port:"<<host.devs[i].msg.image_port<<"  control_port:"<<host.devs[i].msg.control_port<<std::endl;
        }
}
void *scan_loop(void *arg) ;
void discovery_list(Host *host,std::vector<struct sockaddr_in> &addrs){
    for(int i=0;i<host->devs.size();i++){
        if(host->devs[i].addresses.size()==0)
            continue;
        // std::cout<<host->devs[i].id<<std::endl;
        for(int j=0;j<host->devs[i].addresses.size();j++){
            addrs.push_back(host->devs[i].addresses[j].address);
            // std::cout<<"     ip:"<<inet_ntoa(host->devs[i].addresses[j].address.sin_addr)<<" port:"<<ntohs(host->devs[i].addresses[j].address.sin_port)<<std::endl;
        }
    }

}
int discovery_initHost(Host* host,in_port_t port){
    host->port=port;
    host->running=false;
    host->interval=1;
    host->recv_timeout=1;

    host->sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (host->sock < 0) {
        perror("sock error");
        return -1;
    }

    size_t addr_len = sizeof(struct sockaddr_in);
    struct timeval tv;
    tv.tv_sec=host->recv_timeout;
    tv.tv_usec=0;
    int opt=1;
    int ret;
    ret= setsockopt(host->sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    ret= setsockopt(host->sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
    ret= setsockopt(host->sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    if (ret < 0) {
        printf("setsockopt error!\n");
        return -1;
    }

    memset((void*)&host->broadcast_addr, 0, addr_len);
    host->broadcast_addr.sin_family = AF_INET;
    host->broadcast_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    host->broadcast_addr.sin_port = htons(host->port);

    bind(host->sock,(struct sockaddr *)&host->broadcast_addr,addr_len);

    return 0;
}


void discovery_scan_device(Host *host) {
    host->running = true;
    scan(host);
}
void discovery_scan_device_loop(Host *host) {
    host->running = true;
    if(pthread_create(&host->receiver_thread, NULL,scan_loop,host)<0){
        perror("pthread_create");
        exit(1);
    }
    pthread_join(host->receiver_thread, NULL);
}

void checkAlive(Host *host) {
    Message msg;
    for(int i=0;i<host->devs.size();i++){
        for(int j=0;j<host->devs[i].addresses.size();j++){
            if(host->devs[i].addresses[j].scantime+ALIVE_TIME<time(NULL)){
                run_callbacks(host,ADDRESS_NOT_ALIVE,&host->devs[i].addresses[j].address,&host->devs[i].msg);
                host->devs[i].addresses.erase(host->devs[i].addresses.begin()+j);
            }
        }
        if(host->devs[i].addresses.size()==0){
            run_callbacks(host,DEVICE_NOT_ALIVE,NULL,&host->devs[i].msg);
            host->devs.erase(host->devs.begin()+i);
        }
    }
}
void run_callbacks(Host *host,int type,void *arg,void *msg) {
    for (uint16_t i = 0; i < host->callbacks.size(); i++) {
        Callback callback = host->callbacks[i];
        callback(arg,type,msg);
    }
}
int sizecount=0;
void receiver_device(Host *host,int times){
    struct sockaddr_in host_addr;
    size_t addr_len = sizeof(host_addr);
    Message msg;
    time_t start_time= time(NULL);
    while(host->running){
        if(times)
            if(time(NULL)-start_time>2)
                host->running=false;
        ssize_t count = recvfrom(host->sock, &msg, sizeof(msg), 0, (struct sockaddr*)&host_addr, (socklen_t*)&addr_len);
        size_t addr_len = sizeof(struct sockaddr_in);
        // sleep(1);
        if (count >= 0) {
            host->message.host_id=ntohl(msg.host_id);            
            host->message.image_port=ntohs(msg.image_port);
            host->message.control_port=ntohs(msg.control_port);
            ssize_t size = sendto(host->sock, &msg, sizeof(msg), 0, (struct sockaddr*)&host_addr, addr_len);
            int client_id_find=0;
            int client_id_index=0;
            int address_id_index=0;
            for(int i=0;i<host->devs.size();i++){
                if(host->devs[i].msg.host_id==ntohl(msg.host_id)){
                    client_id_index=i;
                    client_id_find=1;
                    break;
                }
            }
            if(!client_id_find){
                Dev dev;
                dev.msg.host_id=host->message.host_id;
                dev.msg.image_port=host->message.image_port;
                dev.msg.control_port=host->message.control_port;
                host->devs.push_back(dev);
                run_callbacks(host, NEW_DEVICE,NULL,NULL);
                Address addr;
                addr.address=host_addr;
                addr.scantime=time(NULL);
                host->devs[host->devs.size()-1].addresses.push_back(addr);
                run_callbacks(host, NEW_ADDRESS,&addr.address,&dev.msg);
                host->devs[host->devs.size()-1].msg.ips.push_back(inet_ntoa(host_addr.sin_addr));
            }else{
                int address_id_find=0;
                for(int j=0;j<host->devs[client_id_index].addresses.size();j++){
                    if(host->devs[client_id_index].addresses[j].address.sin_addr.s_addr==host_addr.sin_addr.s_addr){
                        address_id_index=j;
                        address_id_find=1;
                        break;
                    }
                }
                if(!address_id_find){
                    Address addr;
                    addr.address=host_addr;
                    addr.scantime=time(NULL);
                    host->devs[client_id_index].addresses.push_back(addr);
                    run_callbacks(host, NEW_ADDRESS,&addr.address,&host->devs[client_id_index].msg);
                    host->devs[client_id_index].msg.ips.push_back(inet_ntoa(host_addr.sin_addr));
                }else{
                    host->devs[client_id_index].addresses[address_id_index].scantime=time(NULL);
                }
            }
        }else if(count==-1){
            // std::cout<<"waiting for broadcast message timed out"<<std::endl;
        }
        checkAlive(host);
    }
}
void *scan(void *arg) {
    Host *host = (Host *)arg;
    if(host->running){
        if(host->devs.size()==0){
            receiver_device(host,1);
        }
    }
    close(host->sock);
    return NULL;
}
void *scan_loop(void *arg) {
    Host *host = (Host *)arg;
    if(host->running){
        receiver_device(host,0);
    }
    return NULL;
}

void register_callback(Host *host,Callback callback){
    host->callbacks.push_back(callback);
}


