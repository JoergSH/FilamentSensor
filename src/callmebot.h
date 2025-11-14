/*
 * CallMeBot WhatsApp Notification Module
 * Sends WhatsApp notifications via CallMeBot API
 */

#ifndef CALLMEBOT_H
#define CALLMEBOT_H

#include <Arduino.h>

// Initialize CallMeBot module
void setupCallMeBot();

// Send WhatsApp notification
void sendWhatsAppNotification(const char* message);

// Get/Set CallMeBot settings
bool getCallMeBotEnabled();
void setCallMeBotEnabled(bool enabled);

String getCallMeBotPhone();
void setCallMeBotPhone(const String& phone);

String getCallMeBotApiKey();
void setCallMeBotApiKey(const String& apiKey);

// Notification types
void notifyFilamentError(const char* errorType);
void notifyPrintComplete(const char* filename, unsigned long duration);
void notifyPrintStarted(const char* filename);

#endif // CALLMEBOT_H
