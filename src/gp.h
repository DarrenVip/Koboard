#ifndef GP_H
#define GP_H

/********************************************************************************/
/*                                  T Y P E S                                   */
/********************************************************************************/

/********************/
/*    A N A L O G   */
/********************/

typedef struct {
    uint8_t lx;
    uint8_t ly;
    uint8_t rx;
    uint8_t ry;
} gp_analog_stick_t;

typedef struct {
    uint8_t l2;
    uint8_t r2;
} gp_analog_button_t;

typedef struct {
    gp_analog_stick_t stick;
    gp_analog_button_t button;
} gp_analog_t;


/*********************/
/*   B U T T O N S   */
/*********************/

typedef struct {
    uint8_t options  : 1;
    uint8_t l3       : 1;
    uint8_t r3       : 1;
    uint8_t share    : 1;

    uint8_t up       : 1;
    uint8_t right    : 1;
    uint8_t down     : 1;
    uint8_t left     : 1;

    uint8_t upright  : 1;
    uint8_t upleft   : 1;
    uint8_t downright: 1;
    uint8_t downleft : 1;

    uint8_t l2       : 1;
    uint8_t r2       : 1;
    uint8_t l1       : 1;
    uint8_t r1       : 1;

    uint8_t triangle : 1;
    uint8_t circle   : 1;
    uint8_t cross    : 1;
    uint8_t square   : 1;

    uint8_t ps       : 1;
    uint8_t touchpad : 1;

    uint8_t Y        : 1;
    uint8_t X        : 1;
    uint8_t A        : 1;
    uint8_t B        : 1;
} gp_button_t;


/*******************************/
/*   S T A T U S   F L A G S   */
/*******************************/

typedef struct {
    uint8_t battery;
    uint8_t charging : 1;
    uint8_t audio    : 1;
    uint8_t mic      : 1;
} gp_status_t;


/********************/
/*   S E N S O R S  */
/********************/

typedef struct {
    int16_t z;
} gp_sensor_gyroscope_t;

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} gp_sensor_accelerometer_t;

typedef struct {
    gp_sensor_accelerometer_t accelerometer;
    gp_sensor_gyroscope_t gyroscope;
} gp_sensor_t;


/*******************/
/*    O T H E R    */
/*******************/

typedef struct {
    uint8_t smallRumble;
    uint8_t largeRumble;
    uint8_t r, g, b; // RGB
    uint8_t flashOn;
    uint8_t flashOff; // Time to flash bright/dark (255 = 2.5 seconds)
} gp_cmd_t;

typedef struct {
    gp_button_t button_down;
    gp_button_t button_up;
    gp_analog_t analog_move;
} gp_event_t;

typedef struct {
    gp_analog_t analog;
    gp_button_t button;
} gp_t;


/***************************/
/*    C A L L B A C K S    */
/***************************/

typedef void(*gp_connection_callback_t)( uint8_t is_connected );
typedef void(*gp_connection_object_callback_t)( void *object, uint8_t is_connected );

typedef void(*gp_event_callback_t)( gp_t gp, gp_event_t event );
typedef void(*gp_event_object_callback_t)( void *object, gp_t gp, gp_event_t event );


/********************************************************************************/
/*                             F U N C T I O N S                                */
/********************************************************************************/

bool gpIsConnected();
void gpSetConnectionCallback( gp_connection_callback_t cb );
void gpSetConnectionObjectCallback( void *object, gp_connection_object_callback_t cb );
void gpSetEventCallback( gp_event_callback_t cb );
void gpSetEventObjectCallback( void *object, gp_event_object_callback_t cb );
void gp_connect_event( uint8_t is_connected );


#endif
