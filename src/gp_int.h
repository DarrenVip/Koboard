#ifndef GP_INT_H
#define GP_INT_H
#include "gp.h"

/** ESP-IDF compatibility configuration option choices */
#define IDF_COMPATIBILITY_MASTER_21165ED 3
#define IDF_COMPATIBILITY_MASTER_D9CE0BB 2
#define IDF_COMPATIBILITY_MASTER_21AF1D7 1

#ifndef CONFIG_IDF_COMPATIBILITY
#define CONFIG_IDF_COMPATIBILITY IDF_COMPATIBILITY_MASTER_21AF1D7
#endif

/** Size of the output report buffer for the Dualshock and Navigation controllers */
#define GP_REPORT_BUFFER_SIZE 77
#define GP_HID_BUFFER_SIZE    50

/********************************************************************************/
/*                         S H A R E D   T Y P E S                              */
/********************************************************************************/

enum hid_cmd_code {
    hid_cmd_code_set_report   = 0x50,
    hid_cmd_code_type_output  = 0x02,
    hid_cmd_code_type_feature = 0x03
};

enum hid_cmd_identifier {
    hid_cmd_identifier_gp_enable  = 0xf4,
    hid_cmd_identifier_gp_control = 0x11
};


typedef struct {
  uint8_t code;
  uint8_t identifier;
  uint8_t data[GP_REPORT_BUFFER_SIZE];

} hid_cmd_t;

enum gp_control_packet_index {
    gp_control_packet_index_small_rumble = 5,
    gp_control_packet_index_large_rumble = 6,

    gp_control_packet_index_red = 7,
    gp_control_packet_index_green = 8,
    gp_control_packet_index_blue = 9,

    gp_control_packet_index_flash_on_time = 10,
    gp_control_packet_index_flash_off_time = 11
};


/********************************************************************************/
/*                     C A L L B A C K   F U N C T I O N S                      */
/********************************************************************************/

void gp_connect_event(uint8_t is_connected);
void gp_packet_event( gp_t gp, gp_event_t event );


/********************************************************************************/
/*                      P A R S E R   F U N C T I O N S                         */
/********************************************************************************/

void gp_parse_packet( uint8_t *packet );



/********************************************************************************/
/*                          G A P   F U N C T I O N S                           */
/********************************************************************************/
bool gp_gap_is_connected();
void gp_gap_init_services();
void gp_gap_send_hid( hid_cmd_t *hid_cmd, uint8_t len );

#endif
