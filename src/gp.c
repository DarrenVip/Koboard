#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <esp_system.h>
#include "gp.h"
#include "gp_int.h"

/********************************************************************************/
/*                              C O N S T A N T S                               */
/********************************************************************************/

static const uint8_t hid_cmd_payload_gp_enable[] = { 0x43, 0x02 };


/********************************************************************************/
/*                         L O C A L    V A R I A B L E S                       */
/********************************************************************************/

static gp_connection_callback_t gp_connection_cb = NULL;
static gp_connection_object_callback_t gp_connection_object_cb = NULL;
static void *gp_connection_object = NULL;


static gp_event_callback_t gp_event_cb = NULL;
static gp_event_object_callback_t gp_event_object_cb = NULL;
static void *gp_event_object = NULL;


/********************************************************************************/
/*                      P U B L I C    F U N C T I O N S                        */
/********************************************************************************/



/*******************************************************************************
**
** Function         gpIsConnected
**
** Description      This returns whether a PS4 controller is connected, based
**                  on whether a successful handshake has taken place.
**
**
** Returns          bool
**
*******************************************************************************/
bool gpIsConnected()
{
    return gp_gap_is_connected();
}




/*******************************************************************************
**
** Function         gpSetConnectionCallback
**
** Description      Registers a callback for receiving PS4 controller
**                  connection notifications
**
**
** Returns          void
**
*******************************************************************************/
void gpSetConnectionCallback( gp_connection_callback_t cb )
{
    gp_connection_cb = cb;
}


/*******************************************************************************
**
** Function         gpSetConnectionObjectCallback
**
** Description      Registers a callback for receiving PS4 controller
**                  connection notifications
**
**
** Returns          void
**
*******************************************************************************/
void gpSetConnectionObjectCallback( void *object, gp_connection_object_callback_t cb )
{
    gp_connection_object_cb = cb;
    gp_connection_object = object;
}

/*******************************************************************************
**
** Function         gpSetEventCallback
**
** Description      Registers a callback for receiving PS4 controller events
**
**
** Returns          void
**
*******************************************************************************/
void gpSetEventCallback( gp_event_callback_t cb )
{
    gp_event_cb = cb;
}


/*******************************************************************************
**
** Function         gpSetEventObjectCallback
**
** Description      Registers a callback for receiving PS4 controller events
**
**
** Returns          void
**
*******************************************************************************/
void gpSetEventObjectCallback( void *object, gp_event_object_callback_t cb )
{
    gp_event_object_cb = cb;
    gp_event_object = object;
}



/********************************************************************************/
/*                      L O C A L    F U N C T I O N S                          */
/********************************************************************************/

void gp_connect_event( uint8_t is_connected )
{
    
    if(gp_connection_cb != NULL)
    {
        gp_connection_cb( is_connected );
    }

    if(gp_connection_object_cb != NULL && gp_connection_object != NULL)
    {
        gp_connection_object_cb( gp_connection_object, is_connected );
    }
}


void gp_packet_event( gp_t gp, gp_event_t event )
{
    if(gp_event_cb != NULL)
    {
        gp_event_cb( gp, event );
    }

    if(gp_event_object_cb != NULL && gp_event_object != NULL)
    {
        gp_event_object_cb( gp_event_object, gp, event );
    }
}
