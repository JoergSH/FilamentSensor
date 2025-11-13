/*
 * Filament Sensor Module
 * Handles filament runout and jam detection
 */

#ifndef FILAMENT_SENSOR_H
#define FILAMENT_SENSOR_H

#include <Arduino.h>
#include <atomic>

// Initialize filament sensor
void setupFilamentSensor();

// Check filament sensor status
void checkFilamentSensor();

// Interrupt service routine for filament motion
void filamentMotionISR();

// Check if print head is moving
bool isPrintHeadMoving();

// Enable/disable auto-pause on filament error
void setAutoPauseEnabled(bool enabled);

// Toggle auto-pause on/off
void toggleAutoPause();

// Set motion timeout (in milliseconds)
void setMotionTimeout(unsigned long timeout);

// Get current motion timeout
unsigned long getMotionTimeout();

// Get current filament error state
bool isFilamentErrorDetected();

// Display filament sensor status
void displayFilamentSensorStatus();

// Reset filament sensor state (called when starting/resuming print)
void resetFilamentSensor();

// Get sensor statistics (for web interface)
unsigned long getLastMotionPulse();
unsigned int getMotionPulseCount();
bool getAutoPauseEnabled();

// Set runout pin output state (for testing/control)
void setRunoutPinOutput(bool state);

// Get current runout pin state
String getRunoutPinState();

// Get/Set switch mode (true = direct to RUNOUT_PIN, false = send pause command)
bool getSwitchDirectMode();
void setSwitchDirectMode(bool directMode);
void toggleSwitchMode();

#endif // FILAMENT_SENSOR_H
