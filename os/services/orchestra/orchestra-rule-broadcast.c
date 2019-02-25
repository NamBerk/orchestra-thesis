#include "contiki.h"
#include "orchestra.h"
#include "net/packetbuf.h"
#include "sys/clock.h"
#include "flooding.h"




static uint16_t slotframe_handle = 0; 
static uint16_t channel_offset = 0;  
static struct tsch_slotframe *sf_br;
/*---------------------------------------------------------------------------*/
#if ORCHESTRA_BROADCAST_PERIOD > 0
#define ORCHESTRA_COMMON_SHARED_TYPE LINK_TYPE_NORMAL
#endif
/*---------------------------------------------------------------------------*/
static int
select_packet(uint16_t *slotframe, uint16_t *timeslot)
{
  
  if(packetbuf_attr(PACKETBUF_ATTR_FRAME_TYPE) == FRAME802154_DATAFRAME) { 
   if(slotframe != NULL){
      *slotframe = slotframe_handle;
	   }
   if(timeslot != NULL){
	  *timeslot = 0;
	   }
	}
	   return 1;

}
/*---------------------------------------------------------------------------*/
static void
init(uint16_t sf_handle)
{
	int i;
	slotframe_handle = sf_handle;
	channel_offset = sf_handle; // => 1 / look at the orchestra-conf.h for the order
	sf_br = tsch_schedule_add_slotframe(slotframe_handle , ORCHESTRA_BROADCAST_PERIOD);
	for(i=0; i < (ORCHESTRA_BROADCAST_PERIOD-1)/2; i++){
	tsch_schedule_add_link( sf_br, 
							LINK_OPTION_SHARED|LINK_OPTION_TX|LINK_OPTION_RX,
							ORCHESTRA_COMMON_SHARED_TYPE, 
							&tsch_broadcast_address, 
							i, //=> timeslot
							channel_offset);
	}

	//init_rnc();
	//clock_wait(CLOCK_SECOND); /*here we should wait in order to let NC to be initialized by all nodes then starting it  */
	start_flooding();
	
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