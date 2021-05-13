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
#include "dev/button-sensor.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "sys/log.h"
#include "utils.h"

//For the temperature sensor
#include "dev/i2cmaster.h" 
#include "dev/tmp102.h"

//Take the temperature every 10 seconds
#define TMP102_READ_INTERVAL (10 * CLOCK_SECOND) 

static struct simple_udp_connection udp_conn;
static char delim[] = "$";
int clientId = 0; 

/*---------------------------------------------------------------------------*/
PROCESS(temp_process, "temp_process");
AUTOSTART_PROCESSES(&temp_process);

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
  LOG_INFO("Received response '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");

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
PROCESS_THREAD(temp_process, ev, data)
{
    static struct etimer init_timer;
    static struct etimer read_interval;

    uip_ipaddr_t dest_ipaddr;

    PROCESS_BEGIN();
    
    printf("Starting the temperature sensor\n");

    /* Initialize protocol connection */
    simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,UDP_SERVER_PORT, udp_init_callback);
    etimer_set(&init_timer, SEND_INIT_INTERVAL);

    while(1) {
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&init_timer));
        if((clientId != 0)){
          break;
        }
        if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
          char str[] ="uclEmbedded$init$tempSensor$sen$value";
          simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
          LOG_INFO("Init request sent\n");
        } else {
          LOG_INFO("Not reachable yet\n");
        }
        etimer_restart(&init_timer);
    }

    //Changing the callback to the default one
    simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,UDP_SERVER_PORT, udp_default_callback);

    //initializing the temperature sensor
    int16_t  tempint;
    int16_t  raw;
    uint16_t absraw;
    int16_t  sign;
    tmp102_init();

    char str[100];
    etimer_set(&read_interval, TMP102_READ_INTERVAL); 

    while(1) {  
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&read_interval));  // wait for timer expiration

        /*
        Code provided by zolertia to use de temperature sensor
        http://zolertia.sourceforge.net/wiki/index.php/Mainpage:Contiki_drivers#TMP102_sensor
        */
        sign = 1;
        raw = tmp102_read_temp_raw();  // Reading from the sensor  
        absraw = raw;
        if (raw < 0) { // Perform 2C's if sensor returned negative data
          absraw = (raw ^ 0xFFFF) + 1;
          sign = -1;
        }
        tempint  = (absraw >> 8) * sign; //tempint is the only data we need, we can get more precise information if needed in the future.

        if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) { 
          sprintf(str,"uclEmbedded$info$%d$value$%d",clientId,tempint);
          simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
        }
        else {
          printf("Not reachable yet\n");
        }
        etimer_restart(&read_interval);
    }
    PROCESS_END();
}