
# Mind-Controlled Prosthetic Fingers

### Notes on software development

Due to Coronavirus the hardware for the Final Project could not be completed.  I had originally intended to commute to school once a week in order to continue using the lab and complete the hardware, but as the lab closed I was no longer able to do so.  I developed an **I2C simulator** that forwards all I2C commands to Serial, with a python script simulating the various I2C hardware that needed to be communicated with.  The **Final Project** utilizes this driver, allowing the intended functionality of the hardware to be demonstrated, even though none of the I2C hardware is physically connected to the device.

### Notes on parts

The primary I2C components communicated with are listed below:

**Accelerometer** : *Kionix KX003-1077*
 - [Datasheet](http://kionixfs.kionix.com/en/datasheet/KX003-1077-Specifications-Rev-2.0.pdf)
 - Data output rates from .781Hz to 1600Hz => sample every 1 ms
 - I2C speed up to 3.4MHz
 - Use 12-bit resolution with either 2g or 4g accuracy
 - Use 3.3V
 - 7-bit addr 0x0E/0x0F
 - 8-bit left shift 1, | with 0:write 1:read
 - page 23 for data transfer protocol

**I2C Multiplexer** : *NXP PCA9548AD,118*
 - [Datasheet](https://www.nxp.com/docs/en/data-sheet/PCA9548A.pdf)
 - 3-bit address configuration
 - 1 address byte, 1 control byte
 - 400kHz max SCL
 - 7-bit addr 0xE_ final 3-bits set
 - page 11 for data transfer protocol

**Servo PWM Driver** : *NXP PCA9685PW,118*
 - [Datasheet](https://www.nxp.com/docs/en/data-sheet/PCA9685.pdf)
 - [Usage Schematic](https://cdn-learn.adafruit.com/assets/assets/000/036/269/original/adafruit_products_schem.png?1475858980)
 - [Usage Footprint](https://cdn-learn.adafruit.com/assets/assets/000/036/270/original/adafruit_products_fabprint.png?1475859206)
 - [Driver Code](https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library)
 - Internal clock
 - 6-bit configurable address (1,addr,r/w)
 - page 9 for data transfer protocol


### Boards Designed

The project consists of 5 different boards designed.  One chip for each joint of the finger was designed: the [tip](Hardware/FingerTip), [middle](Hardware/FingerMiddle), and [base](Hardware/FingerBase).  A chip was also designed for the [base of the hand](Hardware/HandChip) containing an I2C mux for controlling the boards on each finger.  The final board designed was to control up to 16 servos (15 needed) that drive a primitive prosthetic hand.
