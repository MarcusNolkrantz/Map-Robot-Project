/*

file: sensor.cpp
author: osklu414
created: 2019-11-14

Interface to sensor module.

*/

#include "sensor.hpp"
#include "logging.hpp"
#include "pc.hpp"
#include <bitset>


Sensor::Sensor(const std::string& file) : Module(file), rx_type(SensorRx::NONE), rx_field(0), latest_measurement(), start_rot(0)
{
    //TRACE("sensor constructor: called with file ", file);
    transmit_identified();
}

Sensor::~Sensor() {}

void Sensor::update()
{
	//TRACE("sensor update:");
	// read transmissions from sensor module
	bool read = true;
	while(read)
	{
		if(rx_type == SensorRx::NONE)
		{
			//TRACE("checking for rx type");
			SensorRx new_rx_type;
			if(serial.read((uint8_t*)&new_rx_type, 1) == 1)
			{
				//TRACE("new rx from sensor module with id ", (int)new_rx_type);
				rx_type = new_rx_type;
				rx_field = 0;
			}
		}

		switch(rx_type)
		{
			case SensorRx::MEASUREMENT:
			{
				//TRACE("receiving measurement byte(s)");
				// new measurement
				const uint8_t n_fields = 6;
				//TRACE("afwpwokdwpodkwpo ", (int)n_fields, " - ", (int)rx_field, " = ", n_fields - rx_field);
				rx_field += serial.read(rx_field_buffer + rx_field, n_fields - rx_field);
				if(rx_field == n_fields)
				{
					int16_t rot_raw = (rx_field_buffer[0] << 8) | rx_field_buffer[1];
					float rot = rot_raw + 720;
					rot -= start_rot;
					uint16_t right = (rx_field_buffer[2] << 8) | rx_field_buffer[3];
					uint16_t left = (rx_field_buffer[4] << 8) | rx_field_buffer[5];
					
					/*
					TRACE("1: ", (bitset<8>)rx_field_buffer[0]," 2: ", (bitset<8>)rx_field_buffer[1]," 3: ", 
					(bitset<8>)rx_field_buffer[2]," 4: ", (bitset<8>)rx_field_buffer[3]," 5: ", (bitset<8>)rx_field_buffer[4]," 6: ", (bitset<8>)rx_field_buffer[5]);
					*/

					
					// store new measurement
					SensorMeasurement measurement{rot, left, right};
					pc->sensor(measurement);
					latest_measurement = measurement;

					//TRACE("received measurement from sensor module");
					//TRACE("rot: ", rot, ", left: ", left, ", right: ", right);
					
					// there might be more to read
					rx_type = SensorRx::NONE;
					rx_field = 0;
				}
				else
				{
					read = false;
				}
				break;
			}
			case SensorRx::COMPETITION:
			{
				// competition mode button pressed
				//TRACE("received competition button press from sensor module");
				// there might be more to read
				competition_callback();
				TRACE("competition button pressed");
				rx_type = SensorRx::NONE;
				break;
			}
			case SensorRx::NONE:
			default:
				read = false;
				rx_type = SensorRx::NONE; // in case received byte is not "correct"
				break;
		}
	}
}


SensorMeasurement Sensor::measurement()
{
	return latest_measurement;
}

void Sensor::transmit_identified()
{
    //TRACE("sensor transmit identified: ");
    uint8_t bytes[1];
    bytes[0] = static_cast<uint8_t>(SensorTx::IDENTIFIED);
    //TRACE("transmitting identified");
    serial.write(bytes, 1);
    //TRACE("transmitting identified done");
}

void Sensor::on_competition(Sensor::CompetitionCallback callback)
{
	competition_callback = callback;
}

void Sensor::init_gyro(float rot){
	start_rot = rot;
}