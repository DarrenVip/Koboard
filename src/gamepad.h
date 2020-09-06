#ifndef gamepad_h
#define gamepad_h

#include <inttypes.h>
#include "Arduino.h"

extern "C" {
  #include  "gp.h"
}


class gamepad {
    public:
        typedef void(*callback_t)();

        gp_t data;
        gp_event_t event;
        gp_cmd_t output;

        gamepad();

        bool begin();

        bool isConnected();


        void attach(callback_t callback);
        void attachOnConnect(callback_t callback);
        void attachOnDisconnect(callback_t callback);

    private:
        static void _event_callback(void *object, gp_t data, gp_event_t event);
        static void _connection_callback(void *object, uint8_t is_connected);

        callback_t _callback_event = nullptr;
        callback_t _callback_connect = nullptr;
        callback_t _callback_disconnect = nullptr;

};

#if !defined(NO_GLOBAL_INSTANCES)
  extern gamepad GP;
#endif

#endif
