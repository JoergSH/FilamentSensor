/*
 * Printer Control Functions
 * High-level control functions for printer operations
 */

#ifndef PRINTER_CONTROL_H
#define PRINTER_CONTROL_H

#include <Arduino.h>

// Start printing a file
void startPrint(String filename);

// Pause current print
void pausePrint();

// Cancel current print
void cancelPrint();

// Resume paused print
void resumePrint();

// Toggle printer light
void toggleLight();

#endif // PRINTER_CONTROL_H
