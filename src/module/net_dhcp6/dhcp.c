#include <kernel/lock/mutex.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/module/module.h>
#include <kernel/mp/event.h>
#include <kernel/mp/thread.h>
#include <kernel/network/layer1.h>
#include <kernel/socket/socket.h>
#include <kernel/timer/timer.h>
#include <kernel/types.h>
#include <net/dhcp6.h>
#include <net/info.h>
#include <net/ip6.h>
#include <net/udp.h>
#define KERNEL_LOG_NAME "net_dhcp6"



static vfs_node_t* KERNEL_INIT_WRITE _net_dhcp6_socket=NULL;
static timer_t* KERNEL_INIT_WRITE _net_dhcp6_timeout_timer=NULL;
static mutex_t* _net_dhcp6_lock;
static u32 _net_dhcp6_current_tid=0;



static net_dhcp6_packet_t* _create_packet(u8 message_type,u32 option_size){
	_net_dhcp6_current_tid++;
	option_size=(option_size+1)&0xfffffffe;
	net_dhcp6_packet_t* out=amm_alloc(sizeof(net_dhcp6_packet_t)+option_size);
	out->type_and_tid=message_type|(_net_dhcp6_current_tid<<8);
	return out;
}



// static void _send_packet(net_dhcp_packet_t* packet,u32 option_size,u32 src_address){
// 	timer_update(_net_dhcp_timeout_timer,DHCP_TIMEOUT_NS,1,1);
// 	net_udp_socket_packet_t* udp_packet=amm_alloc(sizeof(net_udp_socket_packet_t)+sizeof(net_dhcp_packet_t)+option_size);
// 	udp_packet->src_address=src_address;
// 	udp_packet->dst_address=0xffffffff;
// 	udp_packet->src_port=68;
// 	udp_packet->dst_port=67;
// 	udp_packet->length=sizeof(net_dhcp_packet_t)+option_size;
// 	mem_copy(udp_packet->data,packet,udp_packet->length);
// 	amm_dealloc(packet);
// 	socket_push_packet(_net_dhcp_socket,udp_packet,sizeof(net_udp_socket_packet_t)+sizeof(net_dhcp_packet_t)+option_size);
// 	amm_dealloc(udp_packet);
// }



static void _send_solicit_request(void){
	net_dhcp6_packet_t* packet=_create_packet(NET_DHCP6_MESSAGE_TYPE_SOLICIT,10);
	(void)packet;
	// _net_dhcp_offer_address=0;
	// _net_dhcp_offer_server_address=0;
	// if (_net_dhcp_preferred_address){
	// 	net_dhcp_packet_t* packet=_create_packet(10);
	// 	packet->options[0]=NET_DHCP_OPTION_MESSAGE_TYPE;
	// 	packet->options[1]=1;
	// 	packet->options[2]=NET_DHCP_MESSAGE_TYPE_DHCPDISCOVER;
	// 	packet->options[3]=NET_DHCP_OPTION_REQUESTED_IP_ADDRESS;
	// 	packet->options[4]=4;
	// 	packet->options[5]=_net_dhcp_preferred_address>>24;
	// 	packet->options[6]=_net_dhcp_preferred_address>>16;
	// 	packet->options[7]=_net_dhcp_preferred_address>>8;
	// 	packet->options[8]=_net_dhcp_preferred_address;
	// 	packet->options[9]=NET_DHCP_OPTION_END;
	// 	_send_packet(packet,10,0);
	// }
	// else{
	// 	net_dhcp_packet_t* packet=_create_packet(4);
	// 	packet->options[0]=NET_DHCP_OPTION_MESSAGE_TYPE;
	// 	packet->options[1]=1;
	// 	packet->options[2]=NET_DHCP_MESSAGE_TYPE_DHCPDISCOVER;
	// 	packet->options[3]=NET_DHCP_OPTION_END;
	// 	_send_packet(packet,4,0);
	// }
}



static void _rx_thread(void){
	event_t* events[2]={
		_net_dhcp6_timeout_timer->event,
		socket_get_event(_net_dhcp6_socket)
	};
	while (1){
		socket_packet_t* socket_packet=socket_pop_packet(_net_dhcp6_socket,1);
		if (!socket_packet){
			if (!event_await_multiple(events,2)){
				// if (net_info_get_address()){
				// 	LOG("IPv6 lease expired");
				// 	net_info_reset();
				// }
				// else{
				// 	WARN("DHCP timeout");
				// }
				_send_solicit_request();
			}
			continue;
		}
		// net_udp_socket_packet_t* packet=socket_packet->data;
		// mutex_acquire(_net_dhcp_lock);
		// if (packet->length<sizeof(net_dhcp_packet_t)){
		// 	goto _cleanup;
		// }
		// net_dhcp_packet_t* dhcp_packet=(net_dhcp_packet_t*)(packet->data);
		// if (dhcp_packet->op!=NET_DHCP_OP_BOOTREPLY||dhcp_packet->xid!=__builtin_bswap32(_net_dhcp_current_xid)||dhcp_packet->cookie!=__builtin_bswap32(NET_DHCP_COOKIE)){
		// 	goto _cleanup;
		// }
		// u8 op=NET_DHCP_MESSAGE_TYPE_NONE;
		// NET_DHCP_PACKET_ITER_OPTIONS(dhcp_packet){
		// 	if (dhcp_packet->options[i]==NET_DHCP_OPTION_MESSAGE_TYPE&&dhcp_packet->options[i+1]==1){
		// 		op=dhcp_packet->options[i+2];
		// 		break;
		// 	}
		// }
		// if (op==NET_DHCP_MESSAGE_TYPE_DHCPOFFER){
		// 	_net_dhcp_offer_address=__builtin_bswap32(dhcp_packet->yiaddr);
		// 	_net_dhcp_offer_server_address=__builtin_bswap32(dhcp_packet->siaddr);
		// 	INFO("IPv4 offer from %I: %I",_net_dhcp_offer_server_address,_net_dhcp_offer_address);
		// 	net_dhcp_packet_t* packet=_create_packet(16);
		// 	packet->siaddr=dhcp_packet->siaddr;
		// 	packet->options[0]=NET_DHCP_OPTION_MESSAGE_TYPE;
		// 	packet->options[1]=1;
		// 	packet->options[2]=NET_DHCP_MESSAGE_TYPE_DHCPREQUEST;
		// 	packet->options[3]=NET_DHCP_OPTION_REQUESTED_IP_ADDRESS;
		// 	packet->options[4]=4;
		// 	packet->options[5]=_net_dhcp_offer_address>>24;
		// 	packet->options[6]=_net_dhcp_offer_address>>16;
		// 	packet->options[7]=_net_dhcp_offer_address>>8;
		// 	packet->options[8]=_net_dhcp_offer_address;
		// 	packet->options[9]=NET_DHCP_OPTION_SERVER_IDENTIFIER;
		// 	packet->options[10]=4;
		// 	packet->options[11]=_net_dhcp_offer_server_address>>24;
		// 	packet->options[12]=_net_dhcp_offer_server_address>>16;
		// 	packet->options[13]=_net_dhcp_offer_server_address>>8;
		// 	packet->options[14]=_net_dhcp_offer_server_address;
		// 	packet->options[15]=NET_DHCP_OPTION_END;
		// 	_send_packet(packet,16,_net_dhcp_offer_address);
		// }
		// else if (op==NET_DHCP_MESSAGE_TYPE_DHCPACK){
		// 	if (dhcp_packet->yiaddr!=__builtin_bswap32(_net_dhcp_offer_address)||_net_dhcp_offer_server_address!=__builtin_bswap32(dhcp_packet->siaddr)){
		// 		WARN("Received DHCPACK from wrong server");
		// 		goto _cleanup;
		// 	}
		// 	timer_update(_net_dhcp_timeout_timer,0,0,1);
		// 	_net_dhcp_current_xid++; // Ignore any subsequent DHCPACK/DHCPNAK messages
		// 	u32 lease_time=0;
		// 	NET_DHCP_PACKET_ITER_OPTIONS(dhcp_packet){
		// 		u8 type=dhcp_packet->options[i];
		// 		u8 length=dhcp_packet->options[i+1];
		// 		if (type==NET_DHCP_OPTION_SUBNET_MASK&&length==4){
		// 			net_info_set_subnet_mask(__builtin_bswap32(*((u32*)(dhcp_packet->options+i+2))));
		// 		}
		// 		else if (type==NET_DHCP_OPTION_ROUTER&&length>=4){
		// 			net_info_add_router(__builtin_bswap32(*((u32*)(dhcp_packet->options+i+2))));
		// 		}
		// 		else if (type==NET_DHCP_OPTION_DOMAIN_NAME_SERVER&&length>=4){
		// 			net_info_add_dns(__builtin_bswap32(*((u32*)(dhcp_packet->options+i+2))));
		// 		}
		// 		else if (type==NET_DHCP_OPTION_IP_ADDRESS_LEASE_TIME&&length==4){
		// 			lease_time=__builtin_bswap32(*((u32*)(dhcp_packet->options+i+2)));
		// 		}
		// 	}
		// 	net_info_set_address(_net_dhcp_offer_address);
		// 	_net_dhcp_preferred_address=_net_dhcp_offer_address;
		// 	if (!lease_time){
		// 		lease_time=1;
		// 	}
		// 	LOG("IPv4 address: %I",_net_dhcp_offer_address);
		// 	INFO("Subnet mask: %I",net_info_get_subnet_mask());
		// 	INFO("Router:");
		// 	for (const net_info_address_list_entry_t* router=net_info_get_router_entries();router;router=router->next){
		// 		INFO("- %I",router->address);
		// 	}
		// 	INFO("DNS:");
		// 	for (const net_info_address_list_entry_t* router=net_info_get_dns_entries();router;router=router->next){
		// 		INFO("- %I",router->address);
		// 	}
		// 	INFO("Lease time: %us",lease_time);
		// 	_store_config();
		// 	timer_update(_net_dhcp_timeout_timer,lease_time*1000000000ull-DHCP_LEASE_EXPIRY_EARLY_TIME_NS,1,1);
		// }
		// else if (op==NET_DHCP_MESSAGE_TYPE_DHCPNAK){
		// 	_send_discover_request();
		// }
// _cleanup:
		mutex_release(_net_dhcp6_lock);
		socket_dealloc_packet(socket_packet);
	}
}



MODULE_PREINIT(){
	return 0;
}



MODULE_INIT(){
	LOG("Initializing DHCPv6 client...");
	_net_dhcp6_socket=socket_create(SOCKET_DOMAIN_INET6,SOCKET_TYPE_DGRAM,SOCKET_PROTOCOL_UDP);
	// net_udp_address_t local_address={
	// 	0x00000000,
	// 	68
	// };
	// if (!socket_bind(_net_dhcp_socket,&local_address,sizeof(net_udp_address_t))){
	// 	ERROR("Failed to bind DHCPv6 client socket");
	// 	return;
	// }
	_net_dhcp6_timeout_timer=timer_create("net.dhcp6.timeout",0,0);
	_net_dhcp6_lock=mutex_init("net.dhcp6");
	thread_create_kernel_thread(NULL,"net.dhcp6.rx",_rx_thread,0);
}



MODULE_POSTINIT(){
	// _load_config();
#ifndef KERNEL_COVERAGE
	net_dhcp6_negotiate_address();
#endif
}



KERNEL_PUBLIC void net_dhcp6_negotiate_address(void){
	LOG("Negotiating IPv6 address...");
	net_info_reset();
	if (!network_layer1_device){
		ERROR("Unable to negotiate IPv6 address, no network adapter found");
		return;
	}
	mutex_acquire(_net_dhcp6_lock);
	_send_solicit_request();
	mutex_release(_net_dhcp6_lock);
}
