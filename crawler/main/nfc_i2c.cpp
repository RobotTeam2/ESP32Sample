#include "MFRC522.h"

extern "C" void nfc_app_main()
{
    MFRC522 Conector;
    Conector.begin();
    //while(true)
    {
        Conector.readCardSerial();
        Conector.wait();
    }
}
