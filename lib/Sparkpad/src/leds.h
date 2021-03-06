#define TM16XX_CMD_DATA_AUTO 0x40
#define TM16XX_CMD_DATA_READ 0x42 // command to read data used on two wire interfaces of TM1637
#define TM16XX_CMD_DATA_FIXED 0x44
#define TM16XX_CMD_DISPLAY 0x80
#define TM16XX_CMD_ADDRESS 0xC0

#ifdef PROTOTYPE_PCB
#define dataPin 8
#define clockPin 7
#define strobePin 6
#else
#define dataPin 10
#define clockPin 16
#define strobePin 14
#endif

byte grid_array[16] = {};

void send(byte data)
{
    for (int i = 0; i < 8; i++)
    {
        digitalWrite(clockPin, LOW);
        digitalWrite(dataPin, data & 1 ? HIGH : LOW);
        data >>= 1;
        digitalWrite(clockPin, HIGH);
    }
}

void sendCommand(byte cmd)
{
    digitalWrite(strobePin, LOW);
    send(cmd);
    digitalWrite(strobePin, HIGH);
}

void update_tm1638()
{

    sendCommand(TM16XX_CMD_DATA_AUTO);
    digitalWrite(strobePin, LOW);
    send(TM16XX_CMD_ADDRESS);

    for (int i = 0; i < 16; i++)
    {

        byte data = grid_array[i];
        send(data);
    }

    digitalWrite(strobePin, HIGH);
}

void update_led(byte number, byte RGB)
{

    byte seg = number % 6;

    // red LED
    byte grid = number < 6 ? 4 : 10;

    if (RGB & 0x1)
        grid_array[grid] |= 1 << seg;
    else
        grid_array[grid] &= !(1 << seg);

    // green LED
    grid = number < 6 ? 0 : 6;

    if (RGB & 0x2)
        grid_array[grid] |= 1 << seg;
    else
        grid_array[grid] &= !(1 << seg);

    // blue LED
    grid = number < 6 ? 2 : 8;

    if (RGB & 0x4)
        grid_array[grid] |= 1 << seg;
    else
        grid_array[grid] &= !(1 << seg);

    update_tm1638();
}

void update_all_leds(byte RGB)
{

    for (int i = 0; i < 12; i++)
    {

        update_led(i, RGB);
    }
}

void update_bar(byte value)
{

    int mask = (1 << value) - 1;

    grid_array[12] = mask & 0xFF;
    grid_array[13] = (mask >> 8) & 0xFF;

    update_tm1638();

    EEPROM.write(bar_address, bar_value);
}

void setupDisplay(boolean active, byte intensity)
{
    sendCommand(TM16XX_CMD_DISPLAY | (active ? 8 : 0) | min(7, intensity));
}