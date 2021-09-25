#ifndef SLICER_TEST_STREAMS_H
#define SLICER_TEST_STREAMS_H

#include <slicer/modelPartsTypes.h>
#include <string>

class TestStream : public Slicer::Stream<std::string> {
public:
	void Produce(const Consumer & c) override;
};

#endif
