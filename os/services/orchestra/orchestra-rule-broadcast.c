/*Create the new slotframe that is based on broadcast* as the new rule*/
/*New rule is included in the orchestra-conf.h and orchestra.h files*/

#include "contiki.h"
#include "orchestra.h"
#include "net/packetbuf.h"
#include "net/linkaddr.h"
#include "net/ipv6/uip-udp-packet.h"
#include "os/services/rnc/rnc.h"
#include "net/ipv6/tcpip.h"
#include "stdio.h"
#include "net/ipv6/simple-udp.h"
#include "net/ipv6/uip.h"

#define UDP_CLIENT	8765
#define UDP_SERVER	5678
static uint16_t slotframe_handle = 0;
static uint16_t channel_offset = 0;
static struct tsch_slotframe *sf_br;/* new slotframe */
//static broadcast_input_callback current_callback = NULL;

/*---------------------------------------------------------------------------*/
/*void broadcast_set_input_callback(broadcast_input_callback callback) {
	current_callback=callback;
}*/
/*---------------------------------------------------------------------------*/
static void
broad_send(void) {

	start_rnc();
}
/*---------------------------------------------------------------------------*/
static void
broad_receive() {
	

	//if(current_callback != NULL) {
		receiver(c,&(UIP_IP_BUF->srcipaddr),UDP_SERVER,(&UIP_IP_BUF->destipaddr),
		UDP_CLIENT, packetbuf_dataptr(), uip_datalen());
	//}

}
/*---------------------------------------------------------------------------*/
static
uint16_t get_node_timeslot(const linkaddr_t * addr) {


	if (addr != NULL && ORCHESTRA_BROADCAST_PERIOD > 0) {
		return (ORCHESTRA_LINKADDR_HASH(addr)) ;

	} else
		return 0xffff;

}
/*---------------------------------------------------------------------------*/
static int
select_packet(uint16_t *slotframe, uint16_t *timeslot) {

	
	if(packetbuf_attr(PACKETBUF_ATTR_FRAME_TYPE) == FRAME802154_DATAFRAME) {
		broad_receive();

		if(slotframe != NULL) {
			*slotframe = slotframe_handle;

		}
		if(timeslot != NULL) {
			*timeslot = get_node_timeslot(&linkaddr_node_addr);
		}
		return 1;
	}
	return 0;

}
/*---------------------------------------------------------------------------*/
static void
init(uint16_t sf_handle) {

	slotframe_handle = sf_handle;
	channel_offset = sf_handle; 
	sf_br = tsch_schedule_add_slotframe(slotframe_handle , ORCHESTRA_BROADCAST_PERIOD);
	tsch_schedule_add_link(sf_br,
	                       LINK_OPTION_TX|LINK_OPTION_RX,
	                       LINK_TYPE_NORMAL, &tsch_broadcast_address,
	                       get_node_timeslot(&linkaddr_node_addr), channel_offset);
	broad_send();





}
/*---------------------------------------------------------------------------*/
struct orchestra_rule broadcast = {
	init,
	NULL,
	select_packet,
	NULL,
	NULL,
};
/*---------------------------------------------------------------------------*/
