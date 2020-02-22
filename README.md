# Mind-Controlled Prosthetic Fingers

### Notes on parts

**Accelerometer** : *Kionix KX003-1077*
 - (Datasheet)[http://kionixfs.kionix.com/en/datasheet/KX003-1077-Specifications-Rev-2.0.pdf]
 - Data output rates from .781Hz to 1600Hz => sample every 1 ms
 - I2C speed up to 3.4MHz
 - Use 12-bit resolution with either 2g or 4g accuracy
 - Use 3.3V
 - 7-bit addr 0x0E/0x0F
 - 8-bit left shift 1, | with 0:write 1:read
 - page 23 for data transfer protocol
