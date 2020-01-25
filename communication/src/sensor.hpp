/*

file: sensor.hpp
author: osklu414
created: 2019-11-14

Interface to sensor module.

*/

#ifndef SENSOR_HPP
#define SENSOR_HPP

#include <stdint.h>
#include <string>
#include <functional>

#include "module.hpp"
#include "serial.hpp"


enum class SensorTx : uint8_t
{
    IDENTIFIED = 1
};

enum class SensorRx : uint8_t
{
	NONE = 0,
	MEASUREMENT = 7,
	COMPETITION = 2
};

struct SensorMeasurement
{
	float rot;
	uint16_t left;
	uint16_t right;
};

class Sensor : public Module
{
public:
    Sensor(const std::string& file);
    ~Sensor();

	// read sensor measurements
	virtual void update();

	// get latest measurement
	SensorMeasurement measurement();

	using CompetitionCallback = std::function<void()>;

    // set competition button pressed callback
    void on_competition(CompetitionCallback callback);
	
	//Set the new start value for the gyro
	void init_gyro(float rot);


private:
	// confirm that the sensor module has been identified
    void transmit_identified();

	// variables for keeping track of rx:s from sensor module
	SensorRx rx_type;
	const static size_t RX_MAX = 6;
	uint8_t rx_field_buffer[RX_MAX];
	uint8_t rx_field;
	float start_rot;

	// latest measurement
	SensorMeasurement latest_measurement;

	// competition button pressed callback
	CompetitionCallback competition_callback;

};

#endif // SENSOR_HPP
