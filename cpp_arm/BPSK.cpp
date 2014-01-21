/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK.
 *
 * REDHAWK is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */


/**************************************************************************

    This is the component code. This file contains the child class where
    custom functionality can be added to the component. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

 	Source: BPSK.spd.xml
 	Generated on: Thu Aug 22 13:34:33 EDT 2013
 	REDHAWK IDE
 	Version: 1.8.4
 	Build id: R201305151907

**************************************************************************/

#include "BPSK.h"

PREPARE_LOGGING(BPSK_i)

BPSK_i::BPSK_i(const char *uuid, const char *label) : 
    BPSK_base(uuid, label)
{
	m_zeroCrossing = 0;		// Zero crossing counter for the clock signal
	m_signLast = 0;			// The previous sign of the clock
	m_symbolIntegrator = 0;	// The current value of the integration between the clock and data
	m_currXdelta = 0.0;
	m_outputRate = 0.0;
	m_propChanged = false;

	setPropertyChangeListener("Output Rate", this,
			&BPSK_i::propertyChangeListener);
}

BPSK_i::~BPSK_i()
{
}

int BPSK_i::serviceFunction()
{
	bulkio::InFloatPort::dataTransfer *inputData = dataFloat_in_data->getPacket(bulkio::Const::BLOCKING);
	bulkio::InFloatPort::dataTransfer *inputClock = dataFloat_in_clock->getPacket(bulkio::Const::BLOCKING);

	//Check for valid packets
	if (not inputData || not inputClock) {
		if (inputData) {
			delete inputData;
		}

		if (inputClock) {
			delete inputClock;
		}

		return NOOP;
	}

	//Push new SRI
	if (inputData->sriChanged || m_propChanged) {
		m_currXdelta = inputData->SRI.xdelta;
		checkProperties();

		dataShort_out->pushSRI(inputData->SRI);

		m_propChanged = false;
	}

	unsigned int dataSize = inputData->dataBuffer.size();
	unsigned int clockSize = inputClock->dataBuffer.size();
	int signCurrent = 0;
	int symbolLength;

	//Find Symbol length and set output rate
	symbolLength = (int)(1.0 / (inputData->SRI.xdelta * m_outputRate));
	inputData->SRI.xdelta *= symbolLength;

	std::vector<short> outputData;

	if (m_signLast == 0) {
		m_signLast = (inputClock->dataBuffer[0] > 0 ? 1 : -1);
	}

	for (unsigned int i=0; (i < dataSize) && (i < clockSize); ++i) {
		//Find current signal's sign and check for a zero crossing
		signCurrent = (inputClock->dataBuffer[i] > 0 ? 1 : -1);

		if (signCurrent != m_signLast) {
			++m_zeroCrossing;
		}

		//Integrate between clock and data signals
		m_symbolIntegrator += (inputData->dataBuffer[i] * inputClock->dataBuffer[i]);
		m_signLast = signCurrent;

		//If two zero crossings in clock - that's a symbol
		if (m_zeroCrossing >= 2) {
			//If the outputData vector becomes full, break out of function before writing
			//new data out of the vector bounds

			//Check Integral of two signals for symbol. If positive, output 1. If negative, output 0
			//outputData[nout] = (d_symbol_integrator > 0 ? 1 : 0);
			outputData.push_back(m_symbolIntegrator > 0 ? 1 : 0);

			//reset zero counter and integration
			m_symbolIntegrator = 0;
			m_zeroCrossing = 0;
		}
	}

	dataShort_out->pushPacket(outputData, inputData->T, inputData->EOS, inputData->streamID);

	delete inputData; // IMPORTANT: MUST RELEASE THE RECEIVED DATA BLOCKS
	delete inputClock;

	return NORMAL;
}

void BPSK_i::checkProperties()
{
	if (Output_Rate > 1/m_currXdelta) {
		LOG_WARN(BPSK_i, "Output Rate greater than input rate, setting to input rate");
		m_outputRate = 1/m_currXdelta;
	} else {
		m_outputRate = Output_Rate;
	}
}

void BPSK_i::propertyChangeListener(const std::string& prop)
{
	m_propChanged = true;
}
