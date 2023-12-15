/*
 ******************************************************************************
* file      : flight_control.c
* project   : EE 329 P23 Mustang
* details   : I2C/EEPROM
* authors   : John Park
* 			  Chris Hartmann
* 			  Breynyn Northey
*           :
* version   : 0.1
* date      : 2023-DEC-8
* compiler  : STM32CubeIDE Version: 1.13.1 (2023)
* target    : NUCLEO-L4A6ZG
 ******************************************************************************
 * revisions :
* 2023-DEC-8 created.
******************************************************************************
* origins   :
* modified from CubeIDE auto-generated main.c  (c) 2023 STMicro.
* modified from EE329 Lab Manual Sample Code
*****************************************************************************/

#include "main.h"
#include "flight_control.h"
#include <math.h>

// calculating pitch angle from accelerometer data
double get_pitch_angle(int y, int z) {

	double y_dub = (double)y;
	double z_dub = (double)z;

	double cos_angle = y_dub / z_dub;
	// positive -> right
	// negative-> left
	// if user buttons are at bottom (close to user/rear of drone)
	double roll_angle = 90.0 - (acos(cos_angle) * 57.296);
	return roll_angle;
}

// calculating pitch angle from accelerometer data
double get_roll_angle(int x, int z) {
	double x_dub = (double)x;
	double z_dub = (double)z;

	double cos_angle = x_dub / z_dub;
	// pitching up is positive
	// pitching down is negative
	// if user LEDS buttons are at back of the drone
	double pitch_angle = 90.0 - (acos(cos_angle) * 57.296);
	return pitch_angle;

}



// computing output pitch magnitude for servos to be adjusted based upon ptich angle
double comp_pitch(double pitchVar) {
	double servo_pos;
	// pitch angle is postive
	if(pitchVar > 0) {
		if (deadband < fabs(pitchVar) && fabs(pitchVar) < 10) {
			servo_pos = 0.485 * (1.9848 - cos((PI * (pitchVar)) / 180));

		} else if (10 <= fabs(pitchVar) && fabs(pitchVar) < 20) {
			servo_pos = 0.470 * (1.9848 - cos((PI * (pitchVar - 10)) / 180));

		} else if (20 <= fabs(pitchVar) && fabs(pitchVar) < 30) {
			servo_pos = 0.455 * (1.9848 - cos((PI * (pitchVar - 20)) / 180));

		} else if (30 <= fabs(pitchVar) && fabs(pitchVar) < 40) {
			servo_pos = 0.440 * (1.9848 - cos((PI * (pitchVar - 30)) / 180));

		} else if (40 <= fabs(pitchVar) && fabs(pitchVar) < 50) {
			servo_pos = 0.425 * (1.9848 - cos((PI * (pitchVar - 40)) / 180));
		// 0.425 < servo_pos > 0.5
		} else {
			servo_pos = 0.3;
		}

	// pitch angle is negative
	} else {
		if (deadband < fabs(pitchVar) && fabs(pitchVar) < 10) {
			servo_pos = 0.515 * (1.9848 - cos((PI * pitchVar) / 180));

		} else if (10 <= fabs(pitchVar) &&  fabs(pitchVar) < 20) {
			servo_pos = 0.530 * (1.9848 - cos((PI * (fabs(pitchVar) - 10)) / 180));

		} else if (20 <= fabs(pitchVar) &&  fabs(pitchVar) < 30) {
			servo_pos = 0.545 * (1.9848 - cos((PI * (fabs(pitchVar) - 20)) / 180));

		} else if (30 <= fabs(pitchVar) &&  fabs(pitchVar) < 40) {
			servo_pos = 0.560 * (1.9848 - cos((PI * (fabs(pitchVar) - 30)) / 180));

		} else if (40 <= fabs(pitchVar) &&  fabs(pitchVar) < 50) {
			servo_pos = 0.575 * (1.9848 - cos((PI * (fabs(pitchVar) - 40)) / 180));

		// 0.425 < servo_pos > 0.5
		} else {
			servo_pos = 0.7;
		}
	}
	return 1 - servo_pos;
}

// computing output roll magnitude for servos to be adjusted based upon roll angle
double comp_roll(double rollVar) {
	double servo_pos;
	// roll angle is postive
	if(rollVar > 0) {
		if (deadband < fabs(rollVar) && fabs(rollVar) < 10) {
			servo_pos = 0.485 * (1.9848 - cos((PI * rollVar) / 180));

		} else if (10 <= fabs(rollVar) && fabs(rollVar) < 20) {
			servo_pos = 0.470 * (1.9848 - cos((PI * (rollVar - 10)) / 180));

		} else if (20 <= fabs(rollVar) && fabs(rollVar) < 30) {
			servo_pos = 0.455 * (1.9848 - cos((PI * (rollVar - 20)) / 180));

		} else if (30 <= fabs(rollVar) && fabs(rollVar) < 40) {
			servo_pos = 0.440 * (1.9848 - cos((PI * (rollVar - 30)) / 180));

		}else if (40 <= fabs(rollVar) && fabs(rollVar) < 50) {
			servo_pos = 0.425 * (1.9848 - cos((PI * (rollVar - 40)) / 180));
		// 0.425 < servo_pos > 0.5
		} else {
			servo_pos = 0.3;
		}

	}
	// roll angle is negative
	else {
		if (deadband < fabs(rollVar) && fabs(rollVar)  < 10) {
			servo_pos = 0.515 * (1.9848 - cos((PI * rollVar) / 180));

		} else if (10 <= fabs(rollVar) && fabs(rollVar)  < 20) {
			servo_pos = 0.530 * (1.9848 - cos((PI * (fabs(rollVar) - 10)) / 180));

		} else if (20 <= fabs(rollVar) && fabs(rollVar)  < 30) {
			servo_pos = 0.545 * (1.9848 - cos((PI * (fabs(rollVar) - 20)) / 180));

		} else if (30 <= fabs(rollVar) && fabs(rollVar)  < 40) {
			servo_pos = 0.560 * (1.9848 - cos((PI * (fabs(rollVar) - 30)) / 180));

		} else if (40 <= fabs(rollVar) && fabs(rollVar)  < 50) {
			servo_pos = 0.575 * (1.9848 - cos((PI * (fabs(rollVar) - 40)) / 180));
		// 0.425 < servo_pos > 0.5
		} else {
			servo_pos = 0.7;
		}
	}
	return servo_pos;
}

// computing greater magnitude ptich, for inclass demo, so pitch can be easier seen by students
double comp_pitch_demo(double pitchVar) {
	double servo_pos;
	 // pitch angle is postive
	if(pitchVar > 0) {
		if (deadband < fabs(pitchVar) && fabs(pitchVar) < 10) {
			servo_pos = 0.45 * (1.9848 - cos((PI * (pitchVar)) / 180));

		} else if (10 <= fabs(pitchVar) && fabs(pitchVar) < 20) {
			servo_pos = 0.40 * (1.9848 - cos((PI * (pitchVar - 10)) / 180));

		} else if (20 <= fabs(pitchVar) && fabs(pitchVar) < 30) {
			servo_pos = 0.35 * (1.9848 - cos((PI * (pitchVar - 20)) / 180));

		} else if (30 <= fabs(pitchVar) && fabs(pitchVar) < 40) {
			servo_pos = 0.3 * (1.9848 - cos((PI * (pitchVar - 30)) / 180));

		} else if (40 <= fabs(pitchVar) && fabs(pitchVar) < 50) {
			servo_pos = 0.25 * (1.9848 - cos((PI * (pitchVar - 40)) / 180));
		}

	// pitch angle is negative
	} else {
		if (deadband < fabs(pitchVar) && fabs(pitchVar) < 10) {
			servo_pos = 0.55 * (1.9848 - cos((PI * pitchVar) / 180));

		} else if (10 <= fabs(pitchVar) &&  fabs(pitchVar) < 20) {
			servo_pos = 0.6 * (1.9848 - cos((PI * (fabs(pitchVar) - 10)) / 180));

		} else if (20 <= fabs(pitchVar) &&  fabs(pitchVar) < 30) {
			servo_pos = 0.65 * (1.9848 - cos((PI * (fabs(pitchVar) - 20)) / 180));

		} else if (30 <= fabs(pitchVar) &&  fabs(pitchVar) < 40) {
			servo_pos = 0.7 * (1.9848 - cos((PI * (fabs(pitchVar) - 30)) / 180));

		} else if (40 <= fabs(pitchVar) &&  fabs(pitchVar) < 50) {
			servo_pos = 0.575 * (1.9848 - cos((PI * (fabs(pitchVar) - 40)) / 180));
		}
	}
	return servo_pos;
}

// computing greater magnitude roll, for inclass demo, so pitch can be easier seen by students
double comp_roll_demo(double rollVar) {
	double servo_pos;

	// roll angle is postive
	if(rollVar > 0) {
		if (deadband < fabs(rollVar) && fabs(rollVar) < 10) {
			servo_pos = 0.45 * (1.9848 - cos((PI * rollVar) / 180));

		} else if (10 <= fabs(rollVar) && fabs(rollVar) < 20) {
			servo_pos = 0.40 * (1.9848 - cos((PI * (rollVar - 10)) / 180));

		} else if (20 <= fabs(rollVar) && fabs(rollVar) < 30) {
			servo_pos = 0.35 * (1.9848 - cos((PI * (rollVar - 20)) / 180));

		} else if (30 <= fabs(rollVar) && fabs(rollVar) < 40) {
			servo_pos = 0.30 * (1.9848 - cos((PI * (rollVar - 30)) / 180));

		} else if (40 <= fabs(rollVar) && fabs(rollVar) < 50) {
			servo_pos = 0.25 * (1.9848 - cos((PI * (rollVar - 40)) / 180));

		}

	}
	// roll angle is negative
	else {
		if (deadband < fabs(rollVar) && fabs(rollVar)  < 10) {
			servo_pos = 0.55 * (1.9848 - cos((PI * rollVar) / 180));

		} else if (10 <= fabs(rollVar) && fabs(rollVar)  < 20) {
			servo_pos = 0.6 * (1.9848 - cos((PI * (fabs(rollVar) - 10)) / 180));

		} else if (20 <= fabs(rollVar) && fabs(rollVar)  < 30) {
			servo_pos = 0.65 * (1.9848 - cos((PI * (fabs(rollVar) - 20)) / 180));

		} else if (30 <= fabs(rollVar) && fabs(rollVar)  < 40) {
			servo_pos = 0.7 * (1.9848 - cos((PI * (fabs(rollVar) - 30)) / 180));

		} else if (40 <= fabs(rollVar) && fabs(rollVar)  < 50) {
			servo_pos = 0.75 * (1.9848 - cos((PI * (fabs(rollVar) - 40)) / 180));

		}
	}
	return servo_pos;
}

