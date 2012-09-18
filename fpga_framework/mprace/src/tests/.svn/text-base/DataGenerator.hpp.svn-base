/**
 *
 * @file DataGenerator.hpp
 * @author Michael Stapelberg
 * @date 2009-07-13
 *
 */
#ifndef _DATA_GENERATOR_HPP
#define _DATA_GENERATOR_HPP

#include <mprace/Board.h>
#include <stdint.h>

namespace mprace {

#define DG_TRAFFICCLASS_DAQ 1

class ABBDataGenerator {
private:
	mprace::Board *board;
public:
	ABBDataGenerator(mprace::Board *newBoard);

	/**
	 *
	 * Saves a descriptor to the given offset in the descriptor table of the
	 * data generator.
	 *
	 * @param offset Offset to store the descriptor (32-bit aligned)
	 * @param enable Set the enable bit of this descriptor?
	 * @param stop Set the stop bit of this descriptor?
	 * @param traffic_class The traffic class of this descriptor.
	 * @param next_address Offset to the next descriptor (64-bit aligned!)
	 * @param data Data for the DAQ
	 * @todo not only daq data
	 *
	 */
	void saveDescriptor(
		uint32_t offset,
		bool enable,
		bool stop,
		uint8_t traffic_class,
		uint16_t next_address,
		uint16_t delay_count);

	/**
	 *
	 * Saves the data part of a descriptor. You can generate DAQ and CTL
	 * packets, as they have the same meaning of the bits. DLM packets
	 * are not yet implemented. The crc_err flag of the data is always
	 * left 0.
	 *
	 * @param offset Offset of the descriptor + 1
	 * @param sof Start of frame
	 * @param eof End of frame
	 * @param daq_data the DAQ data itself
	 *
	 */
	void saveDAQData(uint32_t offset, bool sof, bool eof, uint16_t daq_data);

	/**
	 *
	 * Stores the given patter of number times 16 bit data
	 * in the DataGenerator and starts it. Data is generated once
	 * if loop is false, and the FIFO is always filled with the
	 * pattern if loop is true.
	 *
	 * This is purely a convenience function which calls saveDescriptor
	 * and saveData for you.
	 *
	 * @param loop true = fill FIFO with the pattern, false = generate once
	 * @param number Number of 16 bit data words
	 * @param data Array of 16 bit data words
	 *
	 */
	void storePattern(bool loop, uint32_t number, uint16_t *data);

	/**
	 *
	 * Blocks until the DataGenerator is no longer busy.
	 *
	 */
	void waitUntilGenerated();

	/**
	 *
	 * Reset the Data Generator and write the necessary bits for stopping
	 * into the first descriptor.
	 *
	 */
	void stop();

	/**
	 *
	 * Reset the Data Generator and write the necessary bits for starting
	 * into the first descriptor.
	 *
	 */
	void start();

};

};

#endif
