#include <user/network.h>
#include <user/syscall.h>
#include <user/types.h>



_Bool network_send(const network_packet_t* packet){
	return _syscall_network_layer2_send(packet,sizeof(network_packet_t));
}



_Bool network_poll(network_packet_t* packet){
	return _syscall_network_layer2_poll(packet,sizeof(network_packet_t));
}



_Bool network_config(network_config_t* config){
	return _syscall_network_layer1_config(config,sizeof(network_config_t));
}
