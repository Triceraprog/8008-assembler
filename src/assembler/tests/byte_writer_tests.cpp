#include "byte_writer.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(ByteWriter, throws_if_address_is_too_high)
{
    std::ostringstream result;
    ByteWriter byte_writer(result, ByteWriter::BINARY);

    ASSERT_THROW(byte_writer.write_byte(1, 0xffff), AddressTooHigh);
}
