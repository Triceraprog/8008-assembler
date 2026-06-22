#include "byte_writer.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(ByteWriter, throws_if_address_is_too_high)
{
    std::ostringstream result;
    ByteWriter byte_writer(result, ByteWriter::BINARY);

    ASSERT_THROW(byte_writer.write_byte(1, 0xffff), AddressTooHigh);
}

TEST(ByteWriter, hex_output_has_correct_format)
{
    std::ostringstream result;
    ByteWriter byte_writer(result, ByteWriter::HEX);
    byte_writer.write_byte(0x01, 0x0000);
    byte_writer.write_end();
    // sum = 0x01 + 0x00 + 0x00 + 0x00 + 0x01 = 0x02, checksum = 0xFE
    ASSERT_THAT(result.str(), Eq(":0100000001FE\n:00000001FF\n"));
}

TEST(ByteWriter, hex_checksum_is_zero_when_byte_sum_is_multiple_of_256)
{
    std::ostringstream result;
    ByteWriter byte_writer(result, ByteWriter::HEX);
    byte_writer.write_byte(0xFF, 0x0000);
    byte_writer.write_end();
    // sum = 0x01 + 0x00 + 0x00 + 0x00 + 0xFF = 0x100, checksum = (0x100 - 0x00) & 0xFF = 0x00
    ASSERT_THAT(result.str(), Eq(":01000000FF00\n:00000001FF\n"));
}
