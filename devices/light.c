/*
  Authors :

  Malicia VAN ERPS
  Adrien LECHANTRE
  Jolan WATHELET

  based on code from contiki-ng examples folder and Z1 official code examples
*/

#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include <stdio.h> 
#include <ctype.h>
#include <stdlib.h>
#include "sys/log.h"
#include "utils.h"

#include "dev/leds.h"


static struct simple_udp_connection udp_conn;
static char delim[] = "$";
int clientId = 0; 

/*---------------------------------------------------------------------------*/
PROCESS(light_process, "light process");
AUTOSTART_PROCESSES(&light_process);

/*---------------------------------------------------------------------------*/
static void
udp_default_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  LOG_INFO("Received '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");
  char* dataCopy = strdup((char *) data);
  char *ptr = strtok(dataCopy, delim);
  if(strcmp(ptr, "uclEmbedded") == 0){
    ptr = strtok(NULL, delim);
    if(strcmp(ptr, "info") == 0){
      ptr = strtok(NULL, delim);
      if(strcmp(ptr, "toggle") == 0){
        leds_toggle(LEDS_ALL);
      }
    }
  }
  free(dataCopy);

}

/*---------------------------------------------------------------------------*/
static void
udp_init_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  LOG_INFO("init caleback Received response '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
  char* dataCopy = strdup((char *) data);
  char *ptr = strtok(dataCopy, delim);
  if(strcmp(ptr, "uclEmbedded") == 0){
    ptr = strtok(NULL, delim);
    if(strcmp(ptr, "ackInit") == 0){
      ptr = strtok(NULL, delim);
      if(isdigit(*ptr)){
        clientId = atoi((char *)ptr);
      }
    }
  }
  free(dataCopy);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(light_process, ev, data)
{
    static struct etimer init_timer;
    uip_ipaddr_t dest_ipaddr;

    PROCESS_BEGIN();
    
    printf("Starting the connected light\n");

    /* Initialize protocol connection */
    simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,UDP_SERVER_PORT, udp_init_callback);
    etimer_set(&init_timer, SEND_INIT_INTERVAL);

    while(1) {
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&init_timer));
        if((clientId != 0)){
          break;
        }
        if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
          char str[] ="uclEmbedded$init$light$rec$toggle";
          simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
          LOG_INFO("Init request sent\n");
        } else {
          LOG_INFO("Not reachable yet\n");
        }
        etimer_restart(&init_timer);
    }

    //Changing the callback to the default one
    simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL, UDP_CLIENT_PORT, udp_default_callback);

    PROCESS_END();
}