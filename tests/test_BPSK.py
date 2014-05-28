#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK.
#
# REDHAWK is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# REDHAWK is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
# for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import unittest
import ossie.utils.testing
import matplotlib.pyplot as plt
from numpy import linspace, pi, sin
from ossie.utils import sb
import time

def display(f, string):
	f.write(string + '\n')
	print string

def displayList(f, string, list):
	f.write(string + "[")
	for i, item in enumerate(list):
		f.write(str(item))
		if i != (len(list) - 1):
			f.write(', ')
	f.write(']\n')
	print string, list

def main():
	f = open('unit_test.log', 'w')

	display(f, "*********************************")
	display(f, "******** BPSK Unit Test *********")
	display(f, "*********************************")

	# Launch the component and the input sources and output sink
	display(f, "\n******* Creating Component ******")
	test_component = sb.launch('../BPSK.spd.xml', execparams={'DEBUG_LEVEL':5})
	clockSource = sb.DataSource()
	dataSource = sb.DataSource()
	dataSink = sb.DataSink()

	# Connect the output of the clock source and the data source
	# to the inputs of the BPSK.  Connect the output of the BPSK
	# to the input of the data sink
	display(f, "\n****** Creating Connections *****")
	clockSource.connect(test_component, providesPortName='clockFloat_in')
	dataSource.connect(test_component, providesPortName='dataFloat_in')
	test_component.connect(dataSink, providesPortName='shortIn')
	display(f, "Connections created")

	display(f, "\n******** Generating Data ********")
	# Generate a simple sine wave for use as the clock and the
	# data
	convenient_time_data = linspace(0, 48*pi, 2400)
	clock = sin(convenient_time_data).tolist()

	display(f, "Single Packet Case...")
	# Use 24 bits to generate a BPSK modulated signal
	single_packet_data = [0,1,0,1,0,0,0,0,0,1,0,0,0,0,1,1,0,1,0,1,0,1,1,1]
	single_packet_keyed_data = []
	i = 0
	for j in single_packet_data:
		for k in range(i, i+100):
			if j == 1:
				single_packet_keyed_data.append(clock[k])
			else:
				single_packet_keyed_data.append(-clock[k])
	
		i += 100

	display(f, "Two Packet Case...")
	# Use a 16 bit packet and an 8 bit packet to generate two
	# BPSK modulated signals using the same clock
	two_packet_data1 = [0,1,0,1,0,0,0,0,0,1,0,0,0,0,1,1]
	two_packet_data2 = [0,1,0,1,0,1,1,1]
	two_packet_keyed_data1 = []
	two_packet_keyed_data2 = []
	i = 0
	for j in two_packet_data1:
		for k in range(i, i+100):
			if j == 1:
				two_packet_keyed_data1.append(clock[k])
			else:
				two_packet_keyed_data1.append(-clock[k])

		i += 100

	for j in two_packet_data2:
		for k in range(i, i+100):
			if j == 1:
				two_packet_keyed_data2.append(clock[k])
			else:
				two_packet_keyed_data2.append(-clock[k])
	
		i += 100

	display(f, "Two Packets, Unaligned Case...")
	# Reuse the one packet from above, but send it as two
	# that aren't lined up on the period boundary
	two_packet_unaligned_keyed_data1 = []
	two_packet_unaligned_keyed_data2 = []
	i = 0
	for j in single_packet_data[:14]:
		for k in range(i, i+100):
			if j == 1:
				two_packet_unaligned_keyed_data1.append(clock[k])
			else:
				two_packet_unaligned_keyed_data1.append(-clock[k])
		
		i += 100
	
	for k in range(i, i+100):
		# Put the first 27 samples into the first packet and the next
		# 73 into the second packet
		if k < (i+27):
			if single_packet_data[14] == 1:
				two_packet_unaligned_keyed_data1.append(clock[k])
			else:
				two_packet_unaligned_keyed_data1.append(-clock[k])
		else:
			if single_packet_data[14] == 1:
				two_packet_unaligned_keyed_data2.append(clock[k])
			else:
				two_packet_unaligned_keyed_data2.append(-clock[k])

	i += 100

	for j in single_packet_data[15:]:
                for k in range(i, i+100):
                        if j == 1:
                                two_packet_unaligned_keyed_data2.append(clock[k])
                        else:
                                two_packet_unaligned_keyed_data2.append(-clock[k])

                i += 100

	display(f, "\n******* Starting Components ******")
	sb.start()
	display(f, "Component started")
	display(f, "** Testing Single Packet Case **")
	# For now, the only accepted output rate of the BPSK is
	# 1187.5 bps.  Since 100 samples of the clock represents
	# a period, this will force the output to be one bit per
	# period
	clockSource.push(clock, False, 'Test', (100 * 1187.5), False, [], None)
	dataSource.push(single_packet_keyed_data, False, 'Test', (100 * 1187.5), False, [], None)
	time.sleep(1)
	
	received_data = dataSink.getData()
	passed_test_1 = True
	test_1_message = ""

	displayList(f, "Received Data:  ",received_data)
        displayList(f, "Original Data:  ", single_packet_data)
	
	# Make sure that the received data and original data
	# are of the same length and that they match
	if len(received_data) == len(single_packet_data):
		for i in range(0, len(received_data)):
			if received_data[i] != single_packet_data[i]:
				passed_test_1 = False
				test_1_message = "received_data[" + str(i) + "] != original_data[" + str(i) + "]"
				break
	else:
		passed_test_1 = False
		test_1_message = "len(received_data) != len(original_data)"

	#******************************************************
	display(f, "\n*** Testing Two Packet Case ****")

	clockSource.push(clock[:len(two_packet_keyed_data1)], False, 'Test', (100 * 1187.5), False, [], None)
	dataSource.push(two_packet_keyed_data1, False, 'Test', (100 * 1187.5), False, [], None)
	time.sleep(1)
	
	received_data = dataSink.getData()
	
	clockSource.push(clock[len(two_packet_keyed_data1):], False, 'Test', (100 * 1187.5), False, [], None)
	dataSource.push(two_packet_keyed_data2, False, 'Test', (100 * 1187.5), False, [], None)
	time.sleep(1)
	
	received_data += dataSink.getData()
	passed_test_2 = True
	test_2_message = ""

	displayList(f, "Received Data:  ", received_data)
	displayList(f, "Original Data1: ", two_packet_data1)
	displayList(f, "Original Data2: ", two_packet_data2)
	
	# Make sure that the received data and original data
        # are of the same length and that they match
	if len(received_data) == (len(two_packet_data1) + len(two_packet_data2)):
		for i in range(0, len(received_data)):
			if i < len(two_packet_data1):
				if received_data[i] != two_packet_data1[i]:
					passed_test_2 = False
					test_2_message = "received_data[" + str(i) + "] != original_data1[" + str(i) + "]"
					break
			else:
				if received_data[i] != two_packet_data2[i-len(two_packet_data1)]:
					passed_test_2 = False
					test_2_message = "received_data[" + str(i) + "] != original_data2[" + str(i - len(two_packet_data1)) + "]"
	else:
		passed_test_2 = False
		test_2_message = "len(received_data) != len(original_data1) + len(original_data2)"

	#******************************************************
	display(f, "\n** Testing Two Packet, Unaligned Case **")
	clockSource.push(clock[:len(two_packet_unaligned_keyed_data1)], False, 'Test', (100 * 1187.5), False, [], None)
	dataSource.push(two_packet_unaligned_keyed_data1, False, 'Test', (100 * 1187.5), False, [], None)
	time.sleep(1)

	received_data = dataSink.getData()

	clockSource.push(clock[len(two_packet_unaligned_keyed_data1):], False, 'Test', (100 * 1187.5), False, [], None)
	dataSource.push(two_packet_unaligned_keyed_data2, False, 'Test', (100 * 1187.5), False, [], None)
	time.sleep(1)

	received_data += dataSink.getData()
	passed_test_3 = True
	test_3_message = ""

	displayList(f, "Received Data:  ", received_data)
	displayList(f, "Original Data:  ", single_packet_data)

	# Make sure that the received data and original data
	# are of the same length and that they match
	if len(received_data) == len(single_packet_data):
		for i in range(0, len(received_data)):
                	if received_data[i] != single_packet_data[i]:
                        	passed_test_3 = False
                                test_3_message = "received_data[" + str(i) + "] != original_data[" + str(i) + "]"
                                break
        else:
                passed_test_3 = False
                test_3_message = "len(received_data) != len(original_data1) + len(original_data2)"

	display(f, "\n******* Stopping Components ******")
	sb.stop()
	display(f, "Components stopped")

	# Display the results of the unit test
	if passed_test_1:
		display(f, "\nSingle Packet Test ...................." + u'\u2714'.encode('utf8'))
	else:
	        display(f, "\nSingle Packet Test ...................." + u'\u2718'.encode('utf8') + '\t' + test_1_message)
	
	if passed_test_2:
		display(f, "Two Packet Test ......................." + u'\u2714'.encode('utf8'))
	else:
		display(f, "Two Packet Test ......................." + u'\u2718'.encode('utf8') + '\t' + test_2_message)
	
	if passed_test_3:
		display(f, "Two Packet, Unaligned Test ............" + u'\u2714'.encode('utf8'))
	else:
		display(f, "Two Packet, Unaligned Test ............" + u'\u2718'.encode('utf8') + '\t' + test_3_message)

	display(f, '\n')
	display(f, "Unit Test Complete")

	f.close()

if __name__ == "__main__":
	main()
