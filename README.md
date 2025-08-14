# Lasercat

## Overview
The Lasercat is a playful toy designed for playful cats. It features a programmable laser pointer controlled by stepper motors, capable of creating various patterns and movements to engage cats.

## Features

### User Interface
- 16x2 LCD display with I2C interface
- Joystick with button for navigation and control
- Menu-driven interface with 7 modes
- Low-power mode for energy efficiency
- Progress bar during homing
- Boundary setting visualization

### Movement Modes
1. **Random Path Mode**
   - Generates smooth, random movement paths
   - Bezier curve interpolation for natural motion
   - Speed fluctuation for dynamic movement
   - Boundary-aware navigation

2. **Quiet Mode**
   - Reduced sound output for sensitive environments
   - Maintains full functionality
   - Persistent state stored in EEPROM

3. **Sleep Mode**
   - Low-power state with minimal power consumption
   - Automatically enters after 10 seconds of inactivity
   - Wakes up with joystick movement
   - LCD backlight control

4. **Boundary Setting Mode**
   - Four-corner boundary system
   - Manual boundary calibration
   - Smart boundary calculation
   - Center point automatic calculation

5. **Custom Path Mode**
   - Real-time joystick control
   - Speed control via joystick
   - Boundary detection and correction
   - Smooth motion interpolation

6. **Demo Mode**
   - Pre-programmed demonstration patterns
   - Shape generation (triangles to hexagons)
   - Square and circle spirals
   - Bouncing patterns
   - Interactive shape demonstrations

7. **Schedule Mode**
   - Automatic operation scheduling
   - Timer-based activation
   - Customizable timing patterns

### Technical Specifications
- **Stepper Motors**
  - Dual-axis control (X and Y)
  - Microstepping capability (1/2 and 1/16)
  - Adjustable speed control (up to 1200 steps/s)
  - Boundary detection with IR sensors
  - Acceleration control
  - Sleep mode for power saving

- **Sensors**
  - IR sensors for boundary detection
  - Joystick with analog and button inputs
  - Button for menu selection
  - Collision detection

- **Audio Feedback**
  - Buzzer for user interaction
  - Click feedback
  - Mode change indicators
  - Quiet mode toggle

- **Path Generation**
  - Bezier curve interpolation
  - Random path generation
  - Shape drawing (polygons, spirals)
  - Bouncing patterns
  - Speed fluctuation

## Hardware Requirements
- Arduino board (compatible with LiquidCrystal_I2C)
- Dual-axis stepper motor system
- 16x2 LCD display with I2C interface (address 0x27)
- Joystick with button (analog pins A3, A2, digital pin 13)
- IR sensors (2) for boundary detection (pins A0, A1)
- Laser pointer module (pin 11)
- Buzzer/speaker (pin 12)
- Power supply (5V for Arduino, 12V for motors)

## Setup
1. Connect all hardware components according to the wiring diagram
2. Upload the firmware using Arduino IDE
3. Calibrate the stepper motors using the boundary setting mode
4. Set movement boundaries using the four-corner system
5. Configure desired movement patterns through the menu system

## Usage
1. Power on the system
2. Use the joystick to navigate the menu
3. Select desired mode using the joystick button
4. Adjust settings as needed
5. System will automatically enter sleep mode after inactivity
6. Wake up system by moving the joystick

## Safety Notes
- Do not point laser at eyes
- Keep out of reach of children
- Use in well-lit environments
- Regularly check hardware connections
- Ensure proper power supply voltage
- Keep pets under supervision

## Project Status
Final version of the project with all features implemented and tested.

## Contributing
This project is part of ME 115 course work. Modifications and improvements are welcome.

## License
This project is intended for educational and personal use only.
