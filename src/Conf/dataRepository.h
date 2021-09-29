/**
 * @Date:   2018-06-08T15:37:07+02:00
 * @Last modified time: 2018-06-08T15:49:33+02:00
 */

/* PH Sensor init data */
#define calibration 1.20 // deviation compensate
#define printInterval 800
#define samplingInterval 20 // PH sampling interval
#define ArrayLenth  40    // times of collection
int pHArray[ArrayLenth];   // Store the average value of the sensor feedback
int pHArrayIndex=0;
/* End PH Sensor */
