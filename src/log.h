#ifndef ESPDMXNODE_LOG_H
#define ESPDMXNODE_LOG_H

enum LOG_LEVEL {
    LOG_DEBUG,
    LOG_INFO,
    LOG_ERROR,
    LOG_OTA,
    LOG_CONFIG,
    LOG_WIFI,
    LOG_ARTNET,
    LOG_E131,
    LOG_OUTPUT,
    LOG_RDM,
    LOG_MQTT
};
const char _log_level_name[][5] = {
    "DEBG",
    "INFO",
    "ERRO",
    "OTA ",
    "CONF",
    "WIFI",
    "ARTN",
    "E131",
    "OUTP",
    "RDM ",
    "MQTT"
};

// -- This should be improved by turning the 2D array in a single buffer and fill all log entries dinamically based on their length
#define LOG_BUFFER_ENTRIES 20
#define LOG_BUFFER_SIZE 256

char logBuffer[LOG_BUFFER_ENTRIES][LOG_BUFFER_SIZE];
char tempLogBuffer[LOG_BUFFER_SIZE];
uint8_t curLogIdx;
uint16_t logEntries;

void log(LOG_LEVEL level, char *msg)
{
    // ALWAYS use the snprintf variant to prevent overflows ffs
    logBuffer[curLogIdx][0] = 0;
    snprintf_P(logBuffer[curLogIdx], LOG_BUFFER_SIZE, PSTR("[%s]: %s"), _log_level_name[level], msg);
    logEntries++;

#ifdef DEBUG
    Serial.println(logBuffer[curLogIdx]);
#endif

    curLogIdx++;
    if (curLogIdx > LOG_BUFFER_ENTRIES - 1)
        curLogIdx = 0;
}
void logf(LOG_LEVEL level, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    tempLogBuffer[0] = 0;
    vsnprintf(tempLogBuffer, LOG_BUFFER_SIZE, format, args);
    va_end(args);
    log(level, tempLogBuffer);
}
void log_P(LOG_LEVEL level, PGM_P msg)
{
    tempLogBuffer[0] = 0;
    strcpy_P(tempLogBuffer, msg);
    log(level, tempLogBuffer);
}
void logf_P(LOG_LEVEL level, PGM_P formatP, ...)
{
    va_list args;
    va_start(args, formatP);
    tempLogBuffer[0] = 0;
    vsnprintf_P(tempLogBuffer, LOG_BUFFER_SIZE, formatP, args);
    va_end(args);
    log(level, tempLogBuffer);
}

#endif
