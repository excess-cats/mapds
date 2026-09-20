#ifndef TELEMETRY_DEFINITIONS_H
#define TELEMETRY_DEFINITIONS_H

#include <cstdint>

/*
 * GPS position
 * Altitude is intentionally kept separate.
 */
struct GPSData
{
    double latitude;
    double longitude;
};

/*
 * Six-axis IMU data.
 */
struct IMUData
{
    float accelerationX;
    float accelerationY;
    float accelerationZ;

    float gyroscopeX;
    float gyroscopeY;
    float gyroscopeZ;
};

/*
 * Magnetometer data.
 */
struct MagnetometerData
{
    float magneticX;
    float magneticY;
    float magneticZ;
};

/*
 * Environmental sensor data.
 */
struct EnvironmentData
{
    float temperatureC;
    float humidityPercent;
    float pressureHpa;
};

/*
 * Date and time information.
 */
struct TimeData
{
    uint16_t year;
    uint8_t month;
    uint8_t day;

    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

/*
 * Battery and power-management information.
 */
struct BatteryData
{
    uint16_t batteryVoltageMv;
    uint8_t batteryPercentage;

    uint16_t vbusVoltageMv;
    uint16_t systemVoltageMv;

    bool batteryConnected;
    bool charging;
    bool discharging;
    bool vbusConnected;
};

/*
 * Reports whether each telemetry source is operating.
 */
struct SensorStatus
{
    bool gpsOperational;
    bool imuOperational;
    bool magnetometerOperational;
    bool environmentOperational;
    bool rtcOperational;
    bool powerManagerOperational;
};

/*
 * Complete shared drone telemetry definition.
 *
 * The Drone, Launcher, and Cockpit Interface use this same structure.
 * Individual modules may use only the fields they need.
 */
struct TelemetryData
{
    GPSData gps;

    double altitudeMeters;

    float groundSpeedMps;
    float courseDegrees;
    uint8_t satelliteCount;
    float hdop;
    bool gpsFixValid;

    TimeData gpsTimeUtc;

    IMUData imu;
    MagnetometerData magnetometer;
    EnvironmentData environment;
    BatteryData battery;

    TimeData rtcTime;

    bool moduleOperational;

    SensorStatus sensorStatus;
};

#endif