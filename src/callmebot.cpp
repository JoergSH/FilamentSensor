/*
 * CallMeBot WhatsApp Notification Module
 */

#include "callmebot.h"
#include <HTTPClient.h>
#include <Preferences.h>

// Preferences namespace
static Preferences preferences;

// CallMeBot settings
static bool callmebotEnabled = false;
static String callmebotPhone = "";
static String callmebotApiKey = "";

// Rate limiting
static unsigned long lastNotificationTime = 0;
static const unsigned long NOTIFICATION_COOLDOWN = 60000;  // 60 seconds between notifications

void setupCallMeBot() {
  // Load settings from flash
  preferences.begin("callmebot", false);

  callmebotEnabled = preferences.getBool("enabled", false);
  callmebotPhone = preferences.getString("phone", "");
  callmebotApiKey = preferences.getString("apiKey", "");

  preferences.end();

  Serial.println("[CALLMEBOT] Module initialized");
  Serial.printf("[CALLMEBOT]   Enabled: %s\n", callmebotEnabled ? "Yes" : "No");
  Serial.printf("[CALLMEBOT]   Phone: %s\n", callmebotPhone.c_str());
  Serial.printf("[CALLMEBOT]   API Key: %s\n", callmebotApiKey.length() > 0 ? "***" : "(not set)");
}

void sendWhatsAppNotification(const char* message) {
  // Check if enabled and configured
  if (!callmebotEnabled) {
    Serial.println("[CALLMEBOT] Notifications disabled");
    return;
  }

  if (callmebotPhone.length() == 0 || callmebotApiKey.length() == 0) {
    Serial.println("[CALLMEBOT] Phone or API key not configured");
    return;
  }

  // Rate limiting - avoid spamming
  unsigned long now = millis();
  if (now - lastNotificationTime < NOTIFICATION_COOLDOWN) {
    Serial.printf("[CALLMEBOT] Rate limit: %lu ms remaining\n",
                  NOTIFICATION_COOLDOWN - (now - lastNotificationTime));
    return;
  }

  // URL encode the message
  String encodedMessage = String(message);
  encodedMessage.replace(" ", "+");
  encodedMessage.replace("\n", "%0A");
  encodedMessage.replace("ä", "%C3%A4");
  encodedMessage.replace("ö", "%C3%B6");
  encodedMessage.replace("ü", "%C3%BC");
  encodedMessage.replace("ß", "%C3%9F");
  encodedMessage.replace("Ä", "%C3%84");
  encodedMessage.replace("Ö", "%C3%96");
  encodedMessage.replace("Ü", "%C3%9C");

  // Build API URL
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + callmebotPhone +
               "&text=" + encodedMessage +
               "&apikey=" + callmebotApiKey;

  Serial.printf("[CALLMEBOT] Sending notification: %s\n", message);

  HTTPClient http;
  http.begin(url);

  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("[CALLMEBOT] Response: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      Serial.println("[CALLMEBOT] ✅ Notification sent successfully");
      lastNotificationTime = now;
    } else {
      Serial.printf("[CALLMEBOT] ❌ Error: %s\n", http.getString().c_str());
    }
  } else {
    Serial.printf("[CALLMEBOT] ❌ Request failed: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}

bool getCallMeBotEnabled() {
  return callmebotEnabled;
}

void setCallMeBotEnabled(bool enabled) {
  callmebotEnabled = enabled;

  preferences.begin("callmebot", false);
  preferences.putBool("enabled", enabled);
  preferences.end();

  Serial.printf("[CALLMEBOT] Enabled: %s\n", enabled ? "Yes" : "No");
}

String getCallMeBotPhone() {
  return callmebotPhone;
}

void setCallMeBotPhone(const String& phone) {
  callmebotPhone = phone;

  preferences.begin("callmebot", false);
  preferences.putString("phone", phone);
  preferences.end();

  Serial.printf("[CALLMEBOT] Phone number set: %s\n", phone.c_str());
}

String getCallMeBotApiKey() {
  return callmebotApiKey;
}

void setCallMeBotApiKey(const String& apiKey) {
  callmebotApiKey = apiKey;

  preferences.begin("callmebot", false);
  preferences.putString("apiKey", apiKey);
  preferences.end();

  Serial.println("[CALLMEBOT] API key updated");
}

void notifyFilamentError(const char* errorType) {
  char message[200];
  snprintf(message, sizeof(message),
           "🚨 Centauri Carbon Alarm!\n\n%s erkannt!\n\nDruck wurde pausiert.",
           errorType);
  sendWhatsAppNotification(message);
}

void notifyPrintComplete(const char* filename, unsigned long duration) {
  char message[200];
  unsigned long hours = duration / 3600000;
  unsigned long minutes = (duration % 3600000) / 60000;

  snprintf(message, sizeof(message),
           "✅ Druck abgeschlossen!\n\nDatei: %s\nDauer: %luh %lumin",
           filename, hours, minutes);
  sendWhatsAppNotification(message);
}

void notifyPrintStarted(const char* filename) {
  char message[200];
  snprintf(message, sizeof(message),
           "🖨️ Druck gestartet\n\nDatei: %s",
           filename);
  sendWhatsAppNotification(message);
}
