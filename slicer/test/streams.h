#pragma once

#include <slicer/modelPartsTypes.h>
#include <string>

class TestStream : public Slicer::Stream<std::string> {
public:
	void Produce(const Consumer & c) override;
};
