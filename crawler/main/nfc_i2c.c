#include "MFRC522.h"

extern "C" void MFRC522_main();
extern "C" void nfc_app_main()
{
    MFRC522_main();
}
