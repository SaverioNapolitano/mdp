#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cstdint>
#include <cassert>
#include <string>
#include <print>
#include <array>
#include <cmath>

using namespace std;
using field_definition = std::array<uint8_t, 3>;

void FitCRC_Get16(uint16_t& crc, uint8_t byte)
{
	static const uint16_t crc_table[16] =
	{
		0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
		0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
	};
	uint16_t tmp;
	// compute checksum of lower four bits of byte
	tmp = crc_table[crc & 0xF];
	crc = (crc >> 4) & 0x0FFF;
	crc = crc ^ tmp ^ crc_table[byte & 0xF];
	// now compute checksum of upper four bits of byte
	tmp = crc_table[crc & 0xF];
	crc = (crc >> 4) & 0x0FFF;
	crc = crc ^ tmp ^ crc_table[(byte >> 4) & 0xF];
}
bool read_header(std::ifstream& is, uint32_t& size, uint16_t& crc) {
	uint8_t header_size = is.get();
	FitCRC_Get16(crc, header_size);
	uint8_t protocol_version = is.get();
	FitCRC_Get16(crc, protocol_version);
	uint16_t profile_version;
	is.read(reinterpret_cast<char*>(&profile_version), 2);
	FitCRC_Get16(crc, static_cast<uint8_t>(profile_version));
	FitCRC_Get16(crc, (profile_version >> 8));
	is.read(reinterpret_cast<char*>(&size), 4);
	FitCRC_Get16(crc, size);
	FitCRC_Get16(crc, (size >> 8));
	FitCRC_Get16(crc, (size >> 16));
	FitCRC_Get16(crc, (size >> 24));
	std::string token(4, ' ');
	is.read(token.data(), 4);
	if (token != ".FIT") {
		return false;
	}
	FitCRC_Get16(crc, token[0]);
	FitCRC_Get16(crc, token[1]);
	FitCRC_Get16(crc, token[2]);
	FitCRC_Get16(crc, token[3]);
	uint16_t actual_crc;
	is.read(reinterpret_cast<char*>(&actual_crc), 2);
	return crc == actual_crc;
}
void read_definition_message(std::ifstream& is, uint16_t& crc, std::map<uint8_t, uint16_t>& local_to_global, std::map<uint8_t, std::vector<field_definition>>& local_field_definition, uint32_t& read) {
	uint8_t header = is.get();
	read++;
	FitCRC_Get16(crc, header);
	uint8_t local_message_type = header & 0xF;
	uint8_t reserved = is.get();
	read++;
	FitCRC_Get16(crc, reserved);
	uint8_t architecture = is.get();
	read++;
	FitCRC_Get16(crc, architecture);
	uint16_t global_message_number;
	is.read(reinterpret_cast<char*>(&global_message_number), 2);
	read += 2;
	local_to_global[local_message_type] = global_message_number;
	FitCRC_Get16(crc, static_cast<uint8_t>(global_message_number));
	FitCRC_Get16(crc, global_message_number >> 8);
	uint8_t num_fields = is.get();
	read++;
	FitCRC_Get16(crc, num_fields);
	for (size_t i = 0; i < num_fields; i++) {
		uint8_t number = is.get();
		read++;
		FitCRC_Get16(crc, number);
		uint8_t size = is.get();
		read++;
		FitCRC_Get16(crc, size);
		uint8_t base_type = is.get();
		read++;
		FitCRC_Get16(crc, base_type);	
		local_field_definition[local_message_type].push_back({ number, size, base_type });
	}
}
void read_data_message(std::ifstream& is, uint16_t& crc, std::map<uint8_t, uint16_t>& local_to_global, std::map<uint8_t, std::vector<field_definition>>& local_field_definition, uint32_t& read) {
	uint8_t header = is.get();
	FitCRC_Get16(crc, header);
	read++;
	uint8_t local_message_type = header & 0xF;
	for (size_t i = 0; i < local_field_definition[local_message_type].size(); i++) {
		uint32_t value = 0;
		if (local_field_definition[local_message_type][i][1] > 4) {
			std::string dummy(local_field_definition[local_message_type][i][1], ' ');
			is.read(dummy.data(), dummy.length());
			read += static_cast<uint32_t>(dummy.length());
			for (auto& c : dummy) {
				FitCRC_Get16(crc, c);
			}
		}
		else {
			is.read(reinterpret_cast<char*>(&value), local_field_definition[local_message_type][i][1]);
			size_t k = 0;
			read += local_field_definition[local_message_type][i][1];
			for (size_t j = 0; j < local_field_definition[local_message_type][i][1]; j++) {
				FitCRC_Get16(crc, (value >> k));
				k += 8;
			}
			if (local_to_global[local_message_type] == 0 && local_field_definition[local_message_type][i][0] == 4) {
				uint32_t time = value;
				std::print("time_created = {}\n", time);
			}
			if (local_to_global[local_message_type] == 19 && local_field_definition[local_message_type][i][0] == 13) {
				uint16_t avg_speed = value;
				double speed = avg_speed * pow(10, -6) * 3600;
				std::print("avg_speed = {:.3f} km/h\n", speed);
			}
		}
		
		
	}
}
int main(int argc, char **argv)
{
	if (argc != 2) {
		return EXIT_FAILURE;
	}
	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return EXIT_FAILURE;
	}
	uint32_t size = 0;
	uint16_t crc = 0;
	if (!read_header(is, size, crc)) {
		return EXIT_FAILURE;
	}
	std::map<uint8_t, uint16_t> local_to_global;
	std::map<uint8_t, std::vector<field_definition>> local_field_definition;
	std::print("Header CRC ok\n");
	uint32_t read = 0;
	crc = 0;
	while (read < size) {
		uint8_t header = is.peek();
		if (((header >> 4) & 0xF) == 4) {
			read_definition_message(is, crc, local_to_global, local_field_definition, read);
		}
		else {
			read_data_message(is, crc, local_to_global, local_field_definition, read);
		}
	}
	uint16_t actual_crc;
	is.read(reinterpret_cast<char*>(&actual_crc), 2);
	if (actual_crc != crc) {
		return EXIT_FAILURE;
	}
	std::print("File CRC ok\n");
	return EXIT_SUCCESS;

}