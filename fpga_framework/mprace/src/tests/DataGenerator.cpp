/**
 *
 * @file DataGenerator.cpp
 * @author Michael Stapelberg
 * @date 2009-07-13
 *
 */
#include <iostream>

#include "DataGenerator.hpp"

#define DG_OFFSET       	(0xC0000 >> 2)
#define DG_CTRL    		(0x00A8  >> 2)
#define DG_CTRL_MASK_BUSY	(0x2)
#define DG_RESET		(0x00A)

using namespace mprace;

ABBDataGenerator::ABBDataGenerator(mprace::Board *newBoard)
{
//	if (newBoard == NULL)
//		throw Exception;

	board = newBoard;

	/* If the data generator bit is not set, it is not present on the card. */
	if ((board->getReg(0x08) & 0x0020) == 0) {
		std::cerr << "No data generator present on card" << std::endl;
		// TODO: throw exception
	}

	/* Clear the first descriptor so that the enable bit will be 0 */
	board->write(DG_OFFSET + 0x0, 0);

	/* Reset data generator */
	std::cout << "resetting data generator..." << std::endl;
	board->setReg(DG_CTRL, DG_RESET);
}

void ABBDataGenerator::saveDescriptor(
		uint32_t offset,
		bool enable,
		bool stop,
		uint8_t traffic_class,
		uint16_t next_address,
		uint16_t delay_count)
{
	uint32_t descriptor = 0x0;

	/* 31 = enable */
	if (enable)
		descriptor |= (1 << 31);

	/* 30 = stop */
	if (stop)
		descriptor |= (1 << 30);

	/* 29:28 = lowest two bits of the descriptor_class */
	descriptor |= ((traffic_class & 0x3) << 28);

	/* 27:16 = next address (in 64-bit mode!) */
	descriptor |= ((next_address & 0xFFF) << 16);

	/* 15:00 = delay count */
	descriptor |= delay_count;

	//std::cout << "Writing descriptor " << std::hex << descriptor << " to " << std::hex << offset << std::endl;

	/* write the descriptor to the given address */
	board->write(DG_OFFSET + offset, descriptor);
}

void ABBDataGenerator::saveDAQData(uint32_t offset, bool sof, bool eof, uint16_t daq_data)
{
	uint32_t data = 0x0;

	if (sof)
		data |= (1 << 17);

	if (eof)
		data |= (1 << 16);

	data |= daq_data;

	//std::cout << "(sof = " << sof << ", eof = " << eof << ")" << std::endl;
	//std::cout << "Writing data " << std::hex << data << " to " << std::hex << offset << std::endl;
	board->write(DG_OFFSET + offset, data);
}

void ABBDataGenerator::storePattern(bool loop, uint32_t number, uint16_t *data)
{
	uint32_t offset = 0x2;

	saveDAQData(1, true, false, data[0]);

	for (unsigned int i = 1; i < number; i++) {
		bool is_last_descriptor = (i == (number-1));
		/* When looping, the next address of the last descriptor needs to be set
		 * to the first descriptor */
		uint16_t next_address = (loop && is_last_descriptor ? 0 : (offset / 2) + 1);

		saveDescriptor(offset,
				false, /* enable is always false but in the first one */
				!loop && is_last_descriptor, /* stop on the last descriptor */
				DG_TRAFFICCLASS_DAQ,
				next_address,
				0 /* delay */);

		saveDAQData(offset+1,
				(i % 4) == 0,
				((i+1) % 4) == 0,
				data[i]);
		offset += 2;
	}
}

void ABBDataGenerator::waitUntilGenerated()
{
	while ((board->getReg(DG_CTRL) & DG_CTRL_MASK_BUSY)) {
		/* block */
		//std::cout << "status = " << board->getReg(DG_CTRL) << std::endl;
	}
}

void ABBDataGenerator::start()
{
	//std::cout << "resetting data generator..." << std::endl;
	board->setReg(DG_CTRL, DG_RESET);

	//std::cout << "FIFO status before start : " << board->getReg(0x24) << std::endl;
	/* Start the Data Generator */
	saveDescriptor(0x0, true, false, DG_TRAFFICCLASS_DAQ, 1, 0);
}

void ABBDataGenerator::stop()
{
	std::cout << "Writing stop descriptor..." << std::endl;
	saveDAQData(0x1, false, true, 0x0);
	saveDescriptor(0x0, false, true, DG_TRAFFICCLASS_DAQ, 1, 0);

	/* Reset data generator */
	std::cout << "resetting data generator..." << std::endl;
	board->setReg(DG_CTRL, DG_RESET);

}
