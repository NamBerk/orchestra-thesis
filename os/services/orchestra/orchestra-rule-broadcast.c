#include "contiki.h"
#include "orchestra.h"
#include "net/packetbuf.h"
//#include "os/services/rnc/rnc.h"
#include "net/linkaddr.h"
#include "net/ipv6/uip-udp-packet.h"
#include "os/services/flooding/flooding.h"
#include "net/ipv6/tcpip.h"

static uint16_t slotframe_handle = 0; 
static uint16_t channel_offset = 0;  
static struct tsch_slotframe *sf_br;
/*#define MAX_PAYLOAD_LEN 120
static struct uip_udp_conn * bcast_conn;
static char buf[MAX_PAYLOAD_LEN];*/
/*---------------------------------------------------------------------------*/
/*#if ORCHESTRA_BROADCAST_PERIOD > 0
#define ORCHESTRA_COMMON_SHARED_TYPE LINK_TYPE_NORMAL
#endif*/
/*---------------------------------------------------------------------------*/
static void
broad_send(void)
{
	start_flooding();
}
/*---------------------------------------------------------------------------*/
static void
broad_receive()
{
  static struct uip_udp_conn *connection ;
  connection = udp_broadcast_new(UIP_HTONS(8765) ,NULL);
  const linkaddr_t *from;
  from = (const linkaddr_t *)PACKETBUF_ADDR_SENDER;
  receiver(connection, from);
	
}
/*---------------------------------------------------------------------------*/
static
uint16_t get_node_timeslot(const linkaddr_t *addr)
{
	
	
    if (ORCHESTRA_BROADCAST_PERIOD > 0){
        return ORCHESTRA_LINKADDR_HASH(addr);
		  
	 }else
		 return 0xffff;
	
}
/*---------------------------------------------------------------------------*/
static int
select_packet(uint16_t *slotframe, uint16_t *timeslot)
{
  if(packetbuf_attr(PACKETBUF_ATTR_FRAME_TYPE) == FRAME802154_DATAFRAME) { 
	broad_receive();
	  
   if(slotframe != NULL){
      *slotframe = slotframe_handle;
	   }
   if(timeslot != NULL){
	  *timeslot = get_node_timeslot(&linkaddr_node_addr);
	   }
	   return 1; 
	}
	   return 0;

}
/*---------------------------------------------------------------------------*/
static void
init(uint16_t sf_handle)
{
	//int i;
	
	slotframe_handle = sf_handle;
	channel_offset = sf_handle; // => 1 / look at the orchestra-conf.h for the order
	sf_br = tsch_schedule_add_slotframe(slotframe_handle , ORCHESTRA_BROADCAST_PERIOD);
	//for(i=0; i < ORCHESTRA_BROADCAST_PERIOD-10;i++){
    tsch_schedule_add_link(sf_br,
                         LINK_OPTION_TX,
                         LINK_TYPE_NORMAL, &tsch_broadcast_address,
                         get_node_timeslot(&linkaddr_node_addr), channel_offset);
	
	//}
  
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