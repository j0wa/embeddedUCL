/*
  Authors :

  Malicia VAN ERPS
  Adrien LECHANTRE
  Jolan WATHELET

  based on code from contiki-ng examples folder and Z1 official code examples
*/

#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"

#include "sys/log.h"

static struct simple_udp_connection udp_conn;
static char delim[] = "$";
int clientIdCounter = 1; 

/*---------------------------------------------------------------------------*/
PROCESS(udp_server_process, "BorderUDPserver");
AUTOSTART_PROCESSES(&udp_server_process);

/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  LOG_INFO("Received request '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");
  char str[100];
  char* dataCopy = strdup((char *) data);
  char *ptr = strtok(dataCopy, delim);
  if(strcmp(ptr, "uclEmbedded") == 0){
    ptr = strtok(NULL, delim);
    if(strcmp(ptr, "init") == 0){
      ptr = strtok(NULL, delim);//don't care for now
      ptr = strtok(NULL, delim);//don't care for now
      sprintf(str,"uclEmbedded$ackInit$%d",clientIdCounter);
      simple_udp_sendto(&udp_conn, str, strlen(str), sender_addr);
      clientIdCounter++;
    }
  }
  free(dataCopy);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  PROCESS_BEGIN();

  /* Initialize DAG root */
  NETSTACK_ROUTING.root_start();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
