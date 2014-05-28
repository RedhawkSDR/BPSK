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

**************************************************************************/

#include "BPSK.h"

PREPARE_LOGGING(BPSK_i)

BPSK_i::BPSK_i(const char *uuid, const char *label) :
    BPSK_base(uuid, label)
{
	m_zeroCrossing = 0;		// Zero crossing counter for the clock signal
	m_signLast = 0;			// The previous sign of the clock
	m_symbolIntegrator = 0;	// The current value of the integration between the clock and data
	m_sriOut = bulkio::sri::create("BPSK_OUT"); //Create output sri object
}

BPSK_i::~BPSK_i()
{
}

int BPSK_i::serviceFunction()
{
	bulkio::InFloatPort::dataTransfer *inputData = dataFloat_in->getPacket(bulkio::Const::BLOCKING);
	bulkio::InFloatPort::dataTransfer *inputClock = clockFloat_in->getPacket(bulkio::Const::BLOCKING);

	std::vector<short> outputData;
	int signCurrent = 0;
	int symbolCount = 0;
	float avgSymbolLength = 0;
	float avgOutputRate = 0;

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

	//Set numSamples to the size of the smaller packet
	unsigned int numSamples = inputData->dataBuffer.size() > inputClock->dataBuffer.size() ?
			inputClock->dataBuffer.size() : inputData->dataBuffer.size();

	//Push new SRI
	if (inputData->sriChanged) {
		m_xdeltaIn = inputClock->SRI.xdelta;
		m_sriOut = inputData->SRI;
		dataShort_out->pushSRI(m_sriOut);
	}

	if (m_signLast == 0) {
		m_signLast = (inputClock->dataBuffer[0] > 0 ? 1 : -1);
	}

	for (unsigned int i=0; i < numSamples; ++i) {
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
			outputData.push_back(m_symbolIntegrator > 0 ? 1 : 0);

			//keep count of total number of symbols
			symbolCount++;

			//reset zero counter and integration
			m_symbolIntegrator = 0;
			m_zeroCrossing = 0;
		}
	}

	//Calculate Symbol Length && Output Rate
	avgSymbolLength = float(numSamples) / symbolCount;
	avgOutputRate = 1.0 / (m_xdeltaIn * avgSymbolLength);

	//Set property value if there has been more than a 1% change
	if (abs(avgSymbolLength - Symbol_Length) > (0.01 * Symbol_Length) ){
		Symbol_Length = avgSymbolLength;
	}
	if (abs(avgOutputRate - Output_Rate) > (0.01 * Output_Rate)){
		Output_Rate = avgOutputRate;
		m_sriOut.xdelta = 1.0f / Output_Rate;
		dataShort_out->pushSRI(m_sriOut);
	}

	if (outputData.size() > 0) {
		dataShort_out->pushPacket(outputData, inputData->T, inputData->EOS, inputData->streamID);
	}

	delete inputData; // IMPORTANT: MUST RELEASE THE RECEIVED DATA BLOCKS
	delete inputClock;

	return NORMAL;
}
