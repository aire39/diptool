#include "VariableLengthCodec.h"

#include <cmath>
#include <algorithm>
#include <utility>
#include <map>
#include <spdlog/spdlog.h>

namespace {

  std::vector<uint8_t> bitstream_to_bytes(const std::vector<bool> & bit_stream)
  {
    std::vector<uint8_t> byte_data;

    if (!bit_stream.empty())
    {
      uint8_t byte = bit_stream[0] ? 0x80 : 0;
      for (size_t i=1; i<bit_stream.size(); i++)
      {
        if ((i % 8) == 0)
        {
          byte_data.emplace_back(byte);
          byte = 0;
        }

        if (bit_stream[i])
        {
          byte |= (0x80 >> (i % 8));
        }
      }

      if (!bit_stream.empty())
      {
        byte_data.emplace_back(byte);
      }
    }

    return byte_data;
  }
}

std::string VariableLengthCodec::Encode(std::vector<uint8_t> image_data, uint32_t width, uint32_t height, int16_t bpp)
{
  encodeMap =  static_cast<char>(width & 0xFF);
  encodeMap += static_cast<char>(width >> 8 & 0xFF);
  encodeMap += static_cast<char>(width >> 16 & 0xFF);
  encodeMap += static_cast<char>(width >> 24 & 0xFF);

  encodeMap += static_cast<char>(height & 0xFF);
  encodeMap += static_cast<char>(height >> 8 & 0xFF);
  encodeMap += static_cast<char>(height >> 16 & 0xFF);
  encodeMap += static_cast<char>(height >> 24 & 0xFF);

  // find all unique values and create a frequency map

  std::map<uint8_t, std::vector<uint8_t>> unique_pixel_value_count;
  for (size_t i=0; i<image_data.size(); i+=bpp)
  {
    uint32_t pixel_value = ((image_data[i + 0] + image_data[i + 1] + image_data[i + 2]) / 3);
    unique_pixel_value_count[pixel_value].push_back(i);
  }

  // with the unique pixel values mapped we need to create another map that maps the pixel values to their
  // probabilities value

  std::vector<std::pair<uint8_t, float>> pixel_probabilities;
  for (const auto & [pv, pv_list] : unique_pixel_value_count)
  {
    pixel_probabilities.emplace_back(pv, static_cast<float>(pv_list.size()) / static_cast<float>(width * height));
  }

  std::sort(pixel_probabilities.begin(), pixel_probabilities.end(), [](const auto & a, const auto & b) -> bool { return (a.second < b.second);});

  // create basic huffman coding map

  std::vector<std::tuple<float, uint8_t, uint8_t, bool, bool>> capture_bit_probabilities;
  for (size_t i=1; i<pixel_probabilities.size(); i++)
  {
    const auto & [prev_pv, prev_pv_prob] = pixel_probabilities[i-1];
    const auto & [cur_pv, cur_pv_prob] = pixel_probabilities[i];

    float accumulated_prob = (prev_pv_prob + cur_pv_prob);

    capture_bit_probabilities.emplace_back(accumulated_prob, prev_pv, cur_pv, prev_pv_prob >= cur_pv_prob, prev_pv_prob < cur_pv_prob);
  }

  // create bit vector table

  std::vector< std::pair<uint8_t, std::vector<bool>> > compression_table;
  std::vector<bool> bit_value_rep;
  for (auto i=static_cast<int32_t>(capture_bit_probabilities.size()-1); i>=0; i--)
  {
    const auto & [accumulated_prob, prev_pv, cur_pv, left_bit, right_bit] = capture_bit_probabilities[i];

    std::vector<bool> set_bits (bit_value_rep);
    set_bits.emplace_back(right_bit);

    bit_value_rep.emplace_back(left_bit);

    compression_table.emplace_back(cur_pv, set_bits);
  }

  compression_table.emplace_back(std::get<1>(capture_bit_probabilities[0]), bit_value_rep);

  uint8_t number_of_compression_tables = compression_table.size();
  encodeMap += static_cast<char>(number_of_compression_tables);

  // add bit vector table to the output

  for (auto [pixel_value, bits] : compression_table)
  {
    encodeMap += static_cast<int8_t>(pixel_value);
    encodeMap += static_cast<int8_t>(bits.size());

    auto bits_to_bytes = bitstream_to_bytes(bits);
    for (const auto & byte : bits_to_bytes)
    {
      encodeMap += static_cast<char>(byte);
    }
  }

  // map the bit vector for each of the pixel values

  std::map<uint8_t, std::vector<bool>> pixel_bit_table_map;
  for (const auto & [pixel_value, bit_vector] : compression_table)
  {
    pixel_bit_table_map[pixel_value] = bit_vector;
  }

  // iterate through all pixels and create the compressed bit stream with the mapped bit vector for each pixel
  // value.
  //
  // note: this is going to explicitly convert all image values into grayscale in this case for now

  std::vector<bool> compressed_image_bitstream;
  for (size_t i=0; i<image_data.size(); i+=bpp)
  {
    uint32_t pixel_value = ((image_data[i + 0] + image_data[i + 1] + image_data[i + 2]) / 3);

    try
    {
      const auto vector_data = pixel_bit_table_map.at(pixel_value);
      compressed_image_bitstream.insert(compressed_image_bitstream.end(), vector_data.begin(), vector_data.end());
    }
    catch (const std::out_of_range & ex)
    {
      spdlog::critical("unable to find key pixel value in map --> what: {}", ex.what());
      encodeMap.clear();
      compressed_image_bitstream.clear();
      break;
    }
  }

  auto compressed_image_data = bitstream_to_bytes(compressed_image_bitstream);
  for (const auto & byte : compressed_image_data)
  {
    encodeMap += static_cast<int8_t>(byte);
  }

  // print the compression size in bytes if there is compressed image data, otherwise clear the encodeMap and print
  // a warning that the compression has failed.

  if (!compressed_image_bitstream.empty())
  {
    spdlog::info("image compression size: {}/{} bytes"
                ,static_cast<uint32_t>(std::ceil((float)compressed_image_bitstream.size() / 8.0f))
                ,compressed_image_data.size());
  }
  else
  {
    spdlog::info("image compression failed");
    encodeMap.clear();
  }

  return encodeMap;
}

std::vector<uint8_t> VariableLengthCodec::Decode(const std::vector<uint8_t> & image_data)
{
  const uint32_t width = *reinterpret_cast<const uint32_t*>(&image_data[0]);
  const uint32_t height = *reinterpret_cast<const uint32_t*>(&image_data[4]);
  std::vector<uint8_t> image (width * height * 4);

  // create bit vector table from file

  std::map<uint8_t, std::vector<bool>> pixel_bit_table_map;
  std::vector<std::pair<uint8_t, std::vector<bool>>> pixel_bit_table_vector;

  uint8_t number_of_bit_tables = image_data[8];
  const uint8_t * table_bit_section = &image_data[9];
  for (size_t i=0; i<number_of_bit_tables; i++)
  {
    uint8_t pixel_value = table_bit_section[0];
    uint8_t table_bit_length = table_bit_section[1];
    auto table_bit_length_in_bytes = static_cast<uint8_t>(std::ceil(static_cast<float>(table_bit_length) / 8.0f));

    const uint8_t * table_data = (table_bit_section + 2);
    std::vector<bool> bit_vector;
    for (size_t j=0; j<table_bit_length_in_bytes; j++)
    {
      uint8_t local_bit_length = std::min(table_bit_length, static_cast<uint8_t>(8));

      for (size_t k = 0; k<local_bit_length; k++)
      {
        bool bit_value = (((0x80 >> k) & *table_data) > 0);
        bit_vector.emplace_back(bit_value);
      }

      table_bit_length -= 8;
      table_data++;
    }

    pixel_bit_table_map[pixel_value] = bit_vector;
    pixel_bit_table_vector.emplace_back(pixel_value, bit_vector);

    table_bit_section += (table_bit_length_in_bytes + 2);
  }

  // sort bit vector table by the bit vector size in ascending order

  std::sort(pixel_bit_table_vector.begin(), pixel_bit_table_vector.end(), [](const std::pair<uint8_t, std::vector<bool>> & a, const std::pair<uint8_t, std::vector<bool>> & b) -> bool {
    return (a.second.size() < b.second.size());
  });

  // convert the bytes to a bit_stream using std::vector<bool> then

  const uint8_t * data_bit_section = table_bit_section;
  const uint8_t * data_bit_section_end = &image_data[image_data.size()-1];

  const size_t data_bit_section_byte_size = (data_bit_section_end - data_bit_section) + 1;
  std::vector<bool> image_data_bit_stream;
  for (size_t i=0; i<data_bit_section_byte_size; i++)
  {
    for (size_t j=0; j<8; j++)
    {
      bool bit_state = ((0x80 >> j) & *data_bit_section) > 0;
      image_data_bit_stream.emplace_back(bit_state);
    }

    data_bit_section++;
  }

  // start converting the bits to the mapped pixel value. Iterate through the stream of bits choosing to capture
  // the max N bits that can be found from the vector bit table.

  std::vector<uint8_t> image_decode_byte_data;
  for (size_t i=0; i<image_data_bit_stream.size();)
  {
    uint32_t test_bit_size = std::min((image_data_bit_stream.size() - i), pixel_bit_table_vector[pixel_bit_table_vector.size()-1].second.size());

    auto it_start = image_data_bit_stream.begin();
    it_start = std::next(it_start, static_cast<int32_t>(i));
    auto it_end = std::next(it_start, test_bit_size);
    std::vector<bool> captured_n_bits (it_start, it_end);

    // iterate through each set of the bit vector table to compare against the capture bits. If any of the vector sets
    // is equal to the captured bits then add the byte associated with that set from the bit vector table and add the
    // number of bits to the iteration variable i. The boolean found_bit_vector will also be true when a valid vector
    // bit set is equal to the captured vector set.

    uint8_t pixel_value = 0;
    bool found_bit_vector = false;
    for (auto j=static_cast<int32_t>(pixel_bit_table_vector.size()-1); j>=0; j--)
    {
      if (captured_n_bits == pixel_bit_table_vector[j].second)
      {
        pixel_value = pixel_bit_table_vector[j].first;
        image_decode_byte_data.emplace_back(pixel_value);
        i += pixel_bit_table_vector[j].second.size();
        found_bit_vector = true;
        break;
      }

      int32_t cur_index = j;
      int32_t next_index = std::max(0, j-1);

      int32_t n_bit_diff = (static_cast<int32_t>(pixel_bit_table_vector[cur_index].second.size()) -
                            static_cast<int32_t>(pixel_bit_table_vector[next_index].second.size()));

      // The size of the captured vector bits will be compared with the same number of bits so the table will iterate
      // through until this occurs. Once this happens a check occurs to make sure the same number of bits in the vector
      // table and captured bits remain the same. This means of the number of bits change or lessen then we should
      // remove the difference in bits from the captured bits buffer as well.

      if ((n_bit_diff > 0) && (pixel_bit_table_vector[cur_index].second.size() <= test_bit_size))
      {
        for (size_t k=0; k<n_bit_diff; k++)
        {
          captured_n_bits.erase(captured_n_bits.end());
        }
      }
    }

    // If the found_bit_vector is still false after going through the vector bit table then this means that we have
    // found all possible values associated with the vector bit table and any left-over bits are just padding therefore
    // the loop should be left.

    if (!found_bit_vector)
    {
      break;
    }
  }

  // assign pixel values to image buffer

  for (size_t i=0; i<image_decode_byte_data.size(); i++)
  {
    uint32_t red_index   = (i * 4) + 0;
    uint32_t green_index = (i * 4) + 1;
    uint32_t blue_index  = (i * 4) + 2;
    uint32_t alpha_index = (i * 4) + 3;

    uint8_t pixel_value = image_decode_byte_data[i];

    image[red_index]   = pixel_value;
    image[green_index] = pixel_value;
    image[blue_index]  = pixel_value;
    image[alpha_index] = 255;
  }

  return image;
}
