#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "gp.h"
#include "gp_int.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "time.h"
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

#define  GP_TAG "GP_PARSER"
#define OFFSET (10)

/********************************************************************************/
/*                            L O C A L    T Y P E S                            */
/********************************************************************************/

time_t prevTime = 0;

enum gp_packet_index {
    gp_packet_index_buttons = 5+OFFSET,
    gp_packet_index_buttonsC = 6+OFFSET,
    gp_packet_index_analog_stick_lx = 1+OFFSET,
    gp_packet_index_analog_stick_ly = 2+OFFSET,
    gp_packet_index_analog_stick_rx = 3+OFFSET,
    gp_packet_index_analog_stick_ry = 4+OFFSET,

   // gp_packet_index_analog_button_l2 = 20,
  //  gp_packet_index_analog_button_r2 = 21,

  //  gp_packet_index_status = 42
};

enum gp_button_mask {
    gp_button_mask_up       = 0,
    gp_button_mask_right    = 2,
    gp_button_mask_down     = 4,
    gp_button_mask_left     = 6,

    gp_button_mask_upright  = 1,
    gp_button_mask_upleft   = 7,
    gp_button_mask_downright= 3,
    gp_button_mask_downleft = 5,

    gp_button_mask_arrows   = 0xf,  //未修改

    gp_button_mask_square   = 24,
    gp_button_mask_cross    = 40,
    gp_button_mask_circle   = 72,
    gp_button_mask_triangle = 136,

    gp_button_mask_Y = 143,
    gp_button_mask_X = 79,
    gp_button_mask_B = 47,
    gp_button_mask_A = 31,

};
enum ps_buttonC_mask
{
    gp_button_mask_l1       = 1,
    gp_button_mask_r1       = 2,
	gp_button_mask_l2       = 4 ,
    gp_button_mask_r2       = 8,

    gp_button_mask_share    =16 ,
    gp_button_mask_options  =32 ,

    gp_button_mask_l3       = 64,
    gp_button_mask_r3       = 128,

 //   gp_button_mask_ps       = 1 << 16,
 //   gp_button_mask_touchpad = 1 << 17


};

enum gp_status_mask {
    gp_status_mask_battery  = 0xf,
    gp_status_mask_charging = 1 << 4,
    gp_status_mask_audio    = 1 << 5,
    gp_status_mask_mic      = 1 << 6,
};


/********************************************************************************/
/*              L O C A L    F U N C T I O N     P R O T O T Y P E S            */
/********************************************************************************/


gp_analog_stick_t gp_parse_packet_analog_stick( uint8_t *packet );
gp_button_t gp_parse_packet_buttons( uint8_t *packet );
gp_event_t gp_parse_event( gp_t prev, gp_t cur );


/********************************************************************************/
/*                         L O C A L    V A R I A B L E S                       */
/********************************************************************************/

static gp_t gp;
static gp_event_callback_t gp_event_cb = NULL;

/********************************************************************************/
/*                      P U B L I C    F U N C T I O N S                        */
/********************************************************************************/
void gp_parser_set_event_cb( gp_event_callback_t cb )
{
    gp_event_cb = cb;
}

void printBytes2Binary(uint8_t *packet, int byteCount) {
    packet += 12;
	int byte = byteCount-4;
	for (; byte >= 0; byte-=4) {
		uint32_t toBinary = *((uint32_t*)&packet[byte]);
        //printf("%d, %d, %d, %d, ", 0xff & (toBinary >> 24), 0xff & (toBinary >> 16), 0xff & (toBinary >> 8), 0xff & toBinary);
		printf("%d : "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN" : %d\n", (byte+3), BYTE_TO_BINARY(toBinary>>24), BYTE_TO_BINARY(toBinary>>16), BYTE_TO_BINARY(toBinary>>8), BYTE_TO_BINARY(toBinary), byte);
	}
	printf("\n");
}

void gp_parse_packet( uint8_t *packet )
{
    gp_t prev_gp = gp;

    //time_t newTime = clock();
    //if (newTime - prevTime > 1000) {
    //    printf("%c", 255);
    //    printBytes2Binary(packet, 44);
    //    printf("Battery = %d\n", gp.status.battery);
    //    prevTime = newTime;
    //}

    gp.button        = gp_parse_packet_buttons(packet);
    gp.analog.stick  = gp_parse_packet_analog_stick(packet);
    //gp.sensor        = gp_parse_packet_sensor(packet);
    //gp.status        = gp_parse_packet_status(packet);

    gp_event_t gp_event = gp_parse_event( prev_gp, gp );

    gp_packet_event( gp, gp_event );

}


/********************************************************************************/
/*                      L O C A L    F U N C T I O N S                          */
/********************************************************************************/

/******************/
/*    E V E N T   */
/******************/
gp_event_t gp_parse_event( gp_t prev, gp_t cur )
{
    gp_event_t gp_event;

    /* Button down events */
    gp_event.button_down.options  = !prev.button.options  && cur.button.options;
    gp_event.button_down.l3       = !prev.button.l3       && cur.button.l3;
    gp_event.button_down.r3       = !prev.button.r3       && cur.button.r3;
    gp_event.button_down.share    = !prev.button.share    && cur.button.share;

    gp_event.button_down.up       = !prev.button.up       && cur.button.up;
    gp_event.button_down.right    = !prev.button.right    && cur.button.right;
    gp_event.button_down.down     = !prev.button.down     && cur.button.down;
    gp_event.button_down.left     = !prev.button.left     && cur.button.left;

    gp_event.button_down.upright  = !prev.button.upright  && cur.button.upright;
    gp_event.button_down.upleft   = !prev.button.upleft   && cur.button.upleft;
    gp_event.button_down.downright= !prev.button.downright&& cur.button.downright;
    gp_event.button_down.downleft = !prev.button.downleft && cur.button.downleft;

    gp_event.button_down.l2       = !prev.button.l2       && cur.button.l2;
    gp_event.button_down.r2       = !prev.button.r2       && cur.button.r2;
    gp_event.button_down.l1       = !prev.button.l1       && cur.button.l1;
    gp_event.button_down.r1       = !prev.button.r1       && cur.button.r1;

    gp_event.button_down.triangle = !prev.button.triangle && cur.button.triangle;
    gp_event.button_down.circle   = !prev.button.circle   && cur.button.circle;
    gp_event.button_down.cross    = !prev.button.cross    && cur.button.cross;
    gp_event.button_down.square   = !prev.button.square   && cur.button.square;

    gp_event.button_down.ps       = !prev.button.ps       && cur.button.ps;
    gp_event.button_down.touchpad = !prev.button.touchpad && cur.button.touchpad;

    gp_event.button_down.Y        = !prev.button.Y        && cur.button.Y;
    gp_event.button_down.X        = !prev.button.X        && cur.button.X;
    gp_event.button_down.A        = !prev.button.A        && cur.button.A;
    gp_event.button_down.B        = !prev.button.B        && cur.button.B;



    /* Button up events */
    gp_event.button_up.options  = prev.button.options  && !cur.button.options;
    gp_event.button_up.l3       = prev.button.l3       && !cur.button.l3;
    gp_event.button_up.r3       = prev.button.r3       && !cur.button.r3;
    gp_event.button_up.share    = prev.button.share    && !cur.button.share;

    gp_event.button_up.up       = prev.button.up       && !cur.button.up;
    gp_event.button_up.right    = prev.button.right    && !cur.button.right;
    gp_event.button_up.down     = prev.button.down     && !cur.button.down;
    gp_event.button_up.left     = prev.button.left     && !cur.button.left;

    gp_event.button_up.upright  = prev.button.upright  && !cur.button.upright;
    gp_event.button_up.upleft   = prev.button.upleft   && !cur.button.upleft;
    gp_event.button_up.downright= prev.button.downright&& !cur.button.downright;
    gp_event.button_up.downleft = prev.button.downleft && !cur.button.downleft;

    gp_event.button_up.l2       = prev.button.l2       && !cur.button.l2;
    gp_event.button_up.r2       = prev.button.r2       && !cur.button.r2;
    gp_event.button_up.l1       = prev.button.l1       && !cur.button.l1;
    gp_event.button_up.r1       = prev.button.r1       && !cur.button.r1;

    gp_event.button_up.triangle = prev.button.triangle && !cur.button.triangle;
    gp_event.button_up.circle   = prev.button.circle   && !cur.button.circle;
    gp_event.button_up.cross    = prev.button.cross    && !cur.button.cross;
    gp_event.button_up.square   = prev.button.square   && !cur.button.square;

    gp_event.button_down.Y        = !prev.button.Y        && !cur.button.Y;
    gp_event.button_down.X        = !prev.button.X        && !cur.button.X;
    gp_event.button_down.A        = !prev.button.A        && !cur.button.A;
    gp_event.button_down.B        = !prev.button.B        && !cur.button.B;

   // gp_event.button_up.ps       = prev.button.ps       && !cur.button.ps;
   // gp_event.button_up.touchpad = prev.button.touchpad && !cur.button.touchpad;

  	gp_event.analog_move.stick.lx = cur.analog.stick.lx != 128;
  	gp_event.analog_move.stick.ly = cur.analog.stick.ly != 128;
  	gp_event.analog_move.stick.rx = cur.analog.stick.rx != 128;
  	gp_event.analog_move.stick.ry = cur.analog.stick.ry != 128;

    return gp_event;
}

/********************/
/*    A N A L O G   */
/********************/
gp_analog_stick_t gp_parse_packet_analog_stick( uint8_t *packet )
{
    gp_analog_stick_t gp_analog_stick;

    //const uint8_t int_offset = 0x80;

    gp_analog_stick.lx = packet[gp_packet_index_analog_stick_lx]; //- int_offset;
  //  printf("gp_analog_stick.lx %d\n",gp_analog_stick.lx);
    gp_analog_stick.ly = packet[gp_packet_index_analog_stick_ly]; //+ int_offset - 1;
   // printf("gp_analog_stick.ly %d\n",gp_analog_stick.ly);
    gp_analog_stick.rx = packet[gp_packet_index_analog_stick_rx] ;//- int_offset;
   // printf(" gp_analog_stick.r %d\n", gp_analog_stick.rx);
    gp_analog_stick.ry = packet[gp_packet_index_analog_stick_ry]; //+ int_offset - 1;
  //  printf("gp_analog_stick.ry %d\n",gp_analog_stick.ry);

    return gp_analog_stick;
}



/*********************/
/*   B U T T O N S   */
/*********************/

gp_button_t gp_parse_packet_buttons( uint8_t *packet )
{
    gp_button_t gp_button;
    uint8_t gp_buttons_raw = packet[gp_packet_index_buttons];
   // printf("gp_buttons_raw is %d\n",gp_buttons_raw);

    gp_button.triangle = (gp_buttons_raw == gp_button_mask_triangle);
    gp_button.circle   = (gp_buttons_raw == gp_button_mask_circle);
    gp_button.cross    = (gp_buttons_raw == gp_button_mask_cross);
    gp_button.square   = (gp_buttons_raw == gp_button_mask_square);

    gp_button.up        = gp_buttons_raw == gp_button_mask_up;
    gp_button.right     = gp_buttons_raw == gp_button_mask_right;
    gp_button.down      = gp_buttons_raw == gp_button_mask_down;
    gp_button.left      = gp_buttons_raw == gp_button_mask_left;

    gp_button.upright   = gp_buttons_raw == gp_button_mask_upright;
    gp_button.upleft    = gp_buttons_raw == gp_button_mask_upleft;
    gp_button.downright = gp_buttons_raw == gp_button_mask_downright;
    gp_button.downleft  = gp_buttons_raw == gp_button_mask_downleft;

    gp_button.A         = gp_buttons_raw == gp_button_mask_A;
    gp_button.B         = gp_buttons_raw == gp_button_mask_B;
    gp_button.X         = gp_buttons_raw == gp_button_mask_X;
    gp_button.Y         = gp_buttons_raw == gp_button_mask_Y;


    uint8_t gp_buttonsC_raw = packet[gp_packet_index_buttonsC];
   // printf("gp_buttonsC_raw is %d\n",gp_buttonsC_raw);
    gp_button.options  = (gp_buttonsC_raw == gp_button_mask_options)  ;
    gp_button.l3       = (gp_buttonsC_raw == gp_button_mask_l3)       ;
    gp_button.r3       = (gp_buttonsC_raw == gp_button_mask_r3)       ;
    gp_button.share    = (gp_buttonsC_raw == gp_button_mask_share)    ;

    gp_button.l2       = (gp_buttonsC_raw == gp_button_mask_l2)       ;
    gp_button.r2       = (gp_buttonsC_raw == gp_button_mask_r2)       ;
    gp_button.l1       = (gp_buttonsC_raw == gp_button_mask_l1)       ;
    gp_button.r1       = (gp_buttonsC_raw == gp_button_mask_r1)       ;



    return gp_button;
}