#define TSCH_SCHEDULE_CONF_WITH_6TISCH_MINIMAL 0
#define TSCH_CONF_WITH_LINK_SELECTOR 1
#define TSCH_CALLBACK_NEW_TIME_SOURCE orchestra_callback_new_time_source
#define TSCH_CALLBACK_PACKET_READY orchestra_callback_packet_ready
#define NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK orchestra_callback_child_added 
#define NETSTACK_CONF_ROUTING_NEIGHBOR_REMOVED_CALLBACK orchestra_callback_child_removed
#define WITH_ORCHESTRA 1
#define ROUTING_CONF_RPL_CLASSIC 1
#define TSCH_CONF_CCA_ENABLED 0
#define ORCHESTRA_CONF_UNICAST_SENDER_BASED 1


//#define LOG_CONF_LEVEL_RPL                         	LOG_LEVEL_INFO

//#define LOG_CONF_LEVEL_TCPIP                       	LOG_LEVEL_INFO	
//#define LOG_CONF_LEVEL_IPV6                        	LOG_LEVEL_INFO
//#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_INFO
#define TSCH_LOG_CONF_PER_SLOT                     1
//#define LOG_CONF_LEVEL_MAC							LOG_LEVEL_INFO





