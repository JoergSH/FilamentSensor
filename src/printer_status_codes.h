#ifndef PRINTER_STATUS_CODES_H
#define PRINTER_STATUS_CODES_H

// Printer Status Codes basierend auf tatsächlichem Printer-Verhalten
// Diese Codes entsprechen den PrintInfo.Status-Werten im JSON
//
// HINWEIS: Der Drucker verwendet unterschiedliche Codes je nach Kontext:
// - Normaler Start vs. nach Pause
// - FDM vs. Resin Drucker-Modi
#define SDCP_PRINT_STATUS_IDLE 0
#define SDCP_PRINT_STATUS_HOMING 1
#define SDCP_PRINT_STATUS_DROPPING 2
#define SDCP_PRINT_STATUS_EXPOSURING 3
#define SDCP_PRINT_STATUS_LIFTING 4
#define SDCP_PRINT_STATUS_PAUSING 5
#define SDCP_PRINT_STATUS_PAUSED 6
#define SDCP_PRINT_STATUS_STOPPING 7
#define SDCP_PRINT_STATUS_STOPPED 8
#define SDCP_PRINT_STATUS_PREPARING 9
#define SDCP_PRINT_STATUS_PAUSED_ALT 10        // Alternative Paused Code
#define SDCP_PRINT_STATUS_PRINTING 11
#define SDCP_PRINT_STATUS_UNKNOWN_12 12
#define SDCP_PRINT_STATUS_PRINTING_ALT 13      // Printing (FDM Mode)
#define SDCP_PRINT_STATUS_UNKNOWN_14 14
#define SDCP_PRINT_STATUS_UNKNOWN_15 15
#define SDCP_PRINT_STATUS_PREPARING_ALT 16     // Preparing (Initial Start)
#define SDCP_PRINT_STATUS_UNKNOWN_17 17
#define SDCP_PRINT_STATUS_PRINTING_RESUME 18   // Printing (After Resume)
#define SDCP_PRINT_STATUS_UNKNOWN_19 19
#define SDCP_PRINT_STATUS_PREPARING_RESUME 20  // Preparing (After Resume)

// Function to get status text
inline const char* getStatusText(int code) {
    switch(code) {
        case SDCP_PRINT_STATUS_IDLE: return "IDLE";
        case SDCP_PRINT_STATUS_HOMING: return "HOMING";
        case SDCP_PRINT_STATUS_DROPPING: return "DROPPING";
        case SDCP_PRINT_STATUS_EXPOSURING: return "EXPOSURING";
        case SDCP_PRINT_STATUS_LIFTING: return "LIFTING";
        case SDCP_PRINT_STATUS_PAUSING: return "PAUSING";
        case SDCP_PRINT_STATUS_PAUSED: return "PAUSED";
        case SDCP_PRINT_STATUS_STOPPING: return "STOPPING";
        case SDCP_PRINT_STATUS_STOPPED: return "STOPPED";
        case SDCP_PRINT_STATUS_PREPARING: return "PREPARING";
        case SDCP_PRINT_STATUS_PAUSED_ALT: return "PAUSED";
        case SDCP_PRINT_STATUS_PRINTING: return "PRINTING";
        case SDCP_PRINT_STATUS_PRINTING_ALT: return "PRINTING";
        case SDCP_PRINT_STATUS_PREPARING_ALT: return "PREPARING";
        case SDCP_PRINT_STATUS_PRINTING_RESUME: return "PRINTING";  // Code 18
        case SDCP_PRINT_STATUS_PREPARING_RESUME: return "PREPARING"; // Code 20
        default: return "UNKNOWN";
    }
}

#endif