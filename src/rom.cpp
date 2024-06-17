#include <fstream>
#include "rom.hpp"
#include <stdio.h>

template <typename IntegerType>
void bitsToInt(IntegerType &result, const char *bits, bool little_endian)
{
  result = 0;
  if (little_endian)
    for (int n = sizeof(result) - 1; n >= 0; n--)
      result = (result << 8) + (uint8_t)bits[n];
  else
    for (int n = 0; n < sizeof(result); n++)
      result = (result << 8) + (uint8_t)bits[n];
}

Rom::Rom(std::string path)
{
  std::ifstream ifs;
  ifs.open(path.c_str(), std::ifstream::binary);

  ifs.seekg(0x100);
  char vals[4];
  ifs.get(vals, 5);
  bitsToInt(entry_point, vals, true);
  for (int i = 0; i < 48; i++)
  {
    logo.push_back(ifs.get());
  }
  for (int i = 0; i < 15; i++)
  {
    unsigned char c = ifs.get();
    _name[i] = c;
  }

  cgb_flag = ifs.get();
  char v[2];
  ifs.get(v, 3);
  bitsToInt(license_code, v, true);
  sgb_flag = ifs.get() & 0x03;
  cartridge_type = ifs.get();
  rom_size = ifs.get();
  ram_size = ifs.get();
  dest_code = ifs.get();
  old_license_code = ifs.get();
  version = ifs.get();
  header_checksum = ifs.get();
  char val[2];
  ifs.get(val, 3);
  bitsToInt(global_checksum, val, true);
  ifs.close();
}

Rom::~Rom()
{
}

void Rom::print_rom() const
{
  printf("title: %s\n", _name);
  printf("cgb_flag: %u\n", cgb_flag);
  printf("license_code: %u\n", license_code);
  printf("sgb_flag: %u\n", sgb_flag);
  printf("cartridge_type: %u\n", cartridge_type);
  printf("rom_size: %u\n", rom_size);
  printf("ram_size: %u\n", ram_size);
  printf("dest_code: %u\n", dest_code);
  printf("old_license_code: %u\n", old_license_code);
  printf("version: %u\n", version);
  printf("header_checksum: %u\n", header_checksum);
  printf("global_checksum: %u\n", global_checksum);
}