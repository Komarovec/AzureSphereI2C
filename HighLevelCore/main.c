#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "constants.h"
#include <applibs/i2c.h>
#include <applibs/log.h>

/*
***** Low-level I2C
*/

/*!
 *  @brief  Write one byte in specified register via I2C
 *  @param _i2c I2C interface
 *  @param  address Device address
 *  @param  reg Address of the register
 *  @param  val Value to be written (MUST BE A BYTE)
 *  @return Number of bytes successfuly written
 */
static int writeReg8(int _i2c, uint8_t address, uint8_t reg, uint8_t val) {
    uint8_t send[2];
    send[0] = reg;
    send[1] = val;
    int er = I2CMaster_Write(_i2c, address, send, sizeof(send));
    return er;
}

/*!
 *  @brief  Read one byte in specified register via I2C
 *  @param _i2c I2C interface
 *  @param  address Device address
 *  @param  reg Address of the register
 *  @param  val pointer to returning value
 *  @return 1 on succes; -1 otherwise
 */
static int readReg8(int _i2c, uint8_t address, uint8_t reg, uint8_t* val) {
    I2CMaster_Write(_i2c, address, &reg, 1);

    uint8_t recv[1];
    if (!I2CMaster_Read(_i2c, address, recv, sizeof(recv))) return -1;

    *val = recv[0];

    return 1;
}

/*!
 *  @brief  Gets the confirming PWM value in registers -> DEBUG
 *  @param  num One of the PWM output pins, from 0 to 15
 *  @desc All values are sent to serial and should be 0 after successful write.
 */
static void readPWM(int _i2c, uint8_t num) {
    int test = 0;
    Log_Debug("PWM reg %d: ", num);
    for (int i = 1; i < 5; i++) {
        readReg8(_i2c, PCA9685_I2C_ADDRESS, (PCA9685_LED0_ON_L + 4 * num) + i, &test);
        Log_Debug("%d; ", test);
    }
    Log_Debug("\n");
}

/*!
 *  @brief  Sets the PWM output of one of the PCA9685 pins
 *  @param _i2c I2C interface
 *  @param  num One of the PWM output pins, from 0 to 15
 *  @param  on At what point in the 4096-part cycle to turn the PWM output ON
 *  @param  off At what point in the 4096-part cycle to turn the PWM output OFF
 *  @return Number of sent bytes
 */
static int setPWM(int _i2c, uint8_t num, uint16_t on, uint16_t off) {
    uint8_t send[5];
    send[0] = PCA9685_LED0_ON_L + 4 * num;
    send[1] = on;
    send[2] = on >> 8;
    send[3] = off;
    send[4] = off >> 8;

    int er = I2CMaster_Write(_i2c, PCA9685_I2C_ADDRESS, send, sizeof(send));
    return er;
}

/*
***** Init functions
*/

/*!
 *  @brief  Setups the i2c interface
 *  @param port i2c port on azure sphere
 *  @param _i2c 
 *  @return 1 if success -1 otherwise
 */
static int initI2C(int port, int *_i2c) {
    *_i2c = I2CMaster_Open(port);
    if (*_i2c == -1) {
        Log_Debug("Cannot open i2c!\n");
    }

    if (I2CMaster_SetBusSpeed(*_i2c, I2C_BUS_SPEED_STANDARD) != 0) {
        Log_Debug("ERROR: Failed to set I2C bus speed: errno=%d (%s)\n",
            errno, strerror(errno));
        return -1;
    }

    if (I2CMaster_SetTimeout(*_i2c, I2C_BUS_TIMEOUT_MS) != 0) {
        Log_Debug("ERROR: I2CMaster_SetTimeout: errno=%d (%s)\n",
            errno, strerror(errno));
        return -1;
    }

    return 1;
}

/*!
 *  @brief  Setups internal registers in PCA9685 for drive
 *  @param _i2c I2C interface
 *  @return 1 on success
 */
static int initBoard(int _i2c) {
    uint8_t mode = 0, err = 0;
    do {
        err = writeReg8(_i2c, PCA9685_I2C_ADDRESS, PCA9685_MODE1, 0);
        if (err <= 0) 
            Log_Debug("Cannot write restart! err: %d\n", err); // err 0 : No bytes sent; -1 : not connected

        // If byte was not received try again
        readReg8(_i2c, PCA9685_I2C_ADDRESS, PCA9685_MODE1, &mode);
        if(mode != 0)
            Log_Debug("Incorrect mode: %d\n Trying again...\n", mode);

    } while (mode != 0);

    uint8_t oldmode;
    readReg8(_i2c, PCA9685_I2C_ADDRESS, PCA9685_MODE1, &oldmode);
    uint8_t newmode = (oldmode & ~MODE1_RESTART) | MODE1_SLEEP; // Prepare sleep byte --> config mode

    // Set prescale
    writeReg8(_i2c, PCA9685_I2C_ADDRESS, PCA9685_MODE1, newmode); // Send sleep byte
    writeReg8(_i2c, PCA9685_I2C_ADDRESS, PCA9685_PRESCALE, 200); // Set prescale to MAX (PWM Period)
    writeReg8(_i2c, PCA9685_I2C_ADDRESS, PCA9685_MODE1, oldmode); // Wake up

    // This sets the MODE1 register to turn on auto increment.
    writeReg8(_i2c, PCA9685_I2C_ADDRESS, PCA9685_MODE1, oldmode | MODE1_RESTART | MODE1_AI);

    return 1;
}

/*
***** High-level interface 
*/

/*!
 *  @brief  Clamps control variables for PWM
 *  @param val Value to clamp
 *  @param on On duty cycle for PWM
 *  @param off Off duty cycle for PWM
 */
void controlVars(uint16_t val, uint16_t *on, uint16_t *off) {
    if (val >= 4096) {
        *on = 4096;
        *off = 0;
    }
    else if (val <= 0) {
        *on = 0;
        *off = 4096;
    }
    else {
        *on = 0;
        *off = val;
    }
}

/*!
 *  @brief  Spins all motors forward
 *  @param _i2c I2C interface
 *  @param val Speed regulation 0 - stop; 4096 - max
 */
void forward(int _i2c, uint16_t val) {
    uint16_t on, off;
    controlVars(val, &on, &off);
    setPWM(_i2c, LEFT1F, on, off);
    setPWM(_i2c, LEFT2F, on, off);
    setPWM(_i2c, LEFT3F, on, off);
    setPWM(_i2c, RIGHT1F, on, off);
    setPWM(_i2c, RIGHT2F, on, off);
    setPWM(_i2c, RIGHT3F, on, off);
}

/*!
 *  @brief  Spins all motors backward
 *  @param _i2c I2C interface
 *  @param val Speed regulation 0 - stop; 4096 - max
 */
void backward(int _i2c, uint16_t val) {
    uint16_t on, off;
    controlVars(val, &on, &off);
    setPWM(_i2c, LEFT1B, on, off);
    setPWM(_i2c, LEFT2B, on, off);
    setPWM(_i2c, LEFT3B, on, off);
    setPWM(_i2c, RIGHT1B, on, off);
    setPWM(_i2c, RIGHT2B, on, off);
    setPWM(_i2c, RIGHT3B, on, off);
}

/*!
 *  @brief  Stops all motors
 *  @param _i2c I2C interface
 */
void stop(int _i2c) {
    for (int i = 0; i < 16; i++) {
        setPWM(_i2c, i, 0, 0);
    }
}

/*!
 *  @brief  Spins left motors forward and right motors backward, turning right
 *  @param _i2c I2C interface
 *  @param val Speed regulation 0 - stop; 4096 - max
 */
void right(int _i2c, uint16_t val) {
    uint16_t on, off;
    controlVars(val, &on, &off);
    setPWM(_i2c, LEFT1F, on, off);
    setPWM(_i2c, LEFT2F, on, off);
    setPWM(_i2c, LEFT3F, on, off);

    setPWM(_i2c, RIGHT1B, on, off);
    setPWM(_i2c, RIGHT2B, on, off);
    setPWM(_i2c, RIGHT3B, on, off);
}

/*!
 *  @brief  Spins right motors forward and left motors backward, turning left
 *  @param _i2c I2C interface
 *  @param val Speed regulation 0 - stop; 4096 - max
 */
void left(int _i2c, uint16_t val) {
    uint16_t on, off;
    controlVars(val, &on, &off);
    setPWM(_i2c, LEFT1B, on, off);
    setPWM(_i2c, LEFT2B, on, off);
    setPWM(_i2c, LEFT3B, on, off);

    setPWM(_i2c, RIGHT1F, on, off);
    setPWM(_i2c, RIGHT2F, on, off);
    setPWM(_i2c, RIGHT3F, on, off);
}

int main() {
    const struct timespec sleepTime = {1, 0};

    // Init I2C and PCA
    int _i2c;
    initI2C(0, &_i2c);
    initBoard(_i2c);

    while (1) {
        // Go slowly forward for 1 sec
        forward(_i2c, 300);
        nanosleep(&sleepTime, NULL);

        // Go slowly backward for 1 sec
        stop(_i2c); // Always clean pins until you take another action!
        backward(_i2c, 200);
        nanosleep(&sleepTime, NULL);

        // Stop for 1 sec
        stop(_i2c);
        nanosleep(&sleepTime, NULL);
    }
}