typedef unsigned char byte;
class MFRC522 {
public:
    MFRC522();
    void begin(void);
    bool available();
    void wait();
    void readCardSerial();
    byte *getCardSerial();
    bool getBlock(byte block, byte keyType, byte *key, byte *returnBlock);
    bool writeBlock(byte block, byte keyType, byte *key, byte *data);
    bool communicate(byte command, byte *sendData, byte sendDataLength, byte *returnData, byte *returnDataLength);
    void write(byte value);
    byte read();
    
private:
    byte cardSerial[4];    
};
