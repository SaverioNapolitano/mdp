#include <fstream>
#include <string>
#include <print>
#include <bit>
#include <vector>
#include <cassert>

template<typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

class bitreader {
	std::istream& is_;
	size_t n_ = 0;
	uint8_t buffer_ = 0;

	uint32_t read_bit() {
		if (n_ == 0) {
			buffer_ = is_.get();
			n_ = 8;
		}
		n_--;
		return (buffer_ >> n_) & 1;
	}

public:

	bitreader(std::ifstream& is):is_(is){}
	~bitreader(){}
	std::istream& operator()(uint32_t& u, size_t n) {
		while (n-- > 0) {
			u = (u << 1) | read_bit();
		}
		return is_;
	}
};

bool read_header(std::istream& is, uint16_t& num_records) {
	std::string name(32, ' ');
	is.read(name.data(), 32);
	uint16_t attributes;
	raw_read(is, attributes);
	uint16_t version;
	raw_read(is, version);
	uint32_t creation_date;
	raw_read(is, creation_date);
	creation_date = std::byteswap(creation_date);
	uint32_t modification_date;
	raw_read(is, modification_date);
	uint32_t last_backup_date;
	raw_read(is, last_backup_date);
	uint32_t modification_number;
	raw_read(is, modification_number);
	uint32_t app_info_id, sort_info_id;
	raw_read(is, app_info_id);
	raw_read(is, sort_info_id);
	std::string type(4, ' ');
	raw_read(is, type[0], 4);
	if (type != "BOOK") {
		return false;
	}
	std::string creator(4, ' ');
	raw_read(is, creator[0], 4);
	if (creator != "MOBI") {
		return false;
	}
	uint32_t unique_id_seed, next_record_list_id;
	raw_read(is, unique_id_seed);
	raw_read(is, next_record_list_id);
	raw_read(is, num_records);
	num_records = std::byteswap(num_records);
	
	std::println("PDB name: {}\nCreation date (s): {}\nType: {}\nCreator: {}\nRecords: {}", name, creation_date, type, creator, num_records);
	return true;
}

void read_record_info_entry(std::istream& is, uint16_t i, uint32_t& record_data_offset) {
	raw_read(is, record_data_offset);
	record_data_offset = std::byteswap(record_data_offset);
	uint8_t record_attributes;
	raw_read(is, record_attributes);
	uint32_t unique_id = 0;
	uint8_t b1, b2, b3;
	raw_read(is, b1);
	raw_read(is, b2);
	raw_read(is, b3);
	unique_id = b3;
	unique_id = (unique_id << 8) | b2;
	unique_id = (unique_id << 8) | b1;

	std::println("{} - offset: {} - id: {}", i, record_data_offset, unique_id);
}

bool read_palm_header(std::istream& is, uint16_t& record_count, uint32_t& text_length) {
	uint16_t compression;
	raw_read(is, compression);
	compression = std::byteswap(compression);
	if (compression != 1 && compression != 2 && compression != 17480) {
		return false;
	}
	uint16_t unused;
	raw_read(is, unused);
	raw_read(is, text_length);
	text_length = std::byteswap(text_length);
	uint16_t record_size, encryption_type, unknown;
	raw_read(is, record_count);
	record_count = std::byteswap(record_count);
	raw_read(is, record_size);
	record_size = std::byteswap(record_size);
	raw_read(is, encryption_type);
	encryption_type = std::byteswap(encryption_type);
	if (encryption_type > 2) {
		return false;
	}
	raw_read(is, unknown);

	std::println("Compression: {}\nTextLength: {}\nRecordCount: {}\nRecordSize: {}\nEncryptionType: {}\n", compression, text_length, record_count, record_size, encryption_type);
	return true;
}

void read_record(std::istream& is, std::ostream& os, size_t& output_bytes, uint32_t text_length, bitreader& br, bool last) {
	
	std::vector<uint8_t> output{};
	if (!last) {
		size_t n = 0;
		while (n < 4096) {
			uint32_t byte = 0;
			br(byte, 8);
			if (byte == 0) {
				break;
			}
			if (byte >= 1 && byte <= 8) {
				for (uint8_t i = 0; i < byte; i++) {
					uint32_t b = 0;
					br(b, 8);
					output.push_back(b);
					n++;
				}
			}
			if (byte >= 0x09 && byte <= 0x7F) {
				output.push_back(byte);
				n++;
			}
			if (byte >= 0x80 && byte <= 0xBF) {
				br(byte, 8);
				uint8_t second_byte = byte & 0xFF;
				uint8_t first_byte = (byte >> 8) & 0xFF;
				assert(((byte >> 14) & 3) == 2);
				uint16_t distance = (byte >> 3) & 2047;
				assert(distance > 0 && distance < 2048);
				uint8_t length = (byte & 7);
				assert(length >= 0 && length <= 7);
				length += 3;
				size_t start = output.size() - distance;
				assert(start < output.size());
				for (uint8_t i = 0; i < length; i++) {
					output.push_back(output[start++]);
					n++;
				}
			}
			if (byte >= 0xC0 && byte <= 0xFF) {
				output.push_back(0x20);
				byte = byte & 0x7F;
				output.push_back(byte);
				n += 2;
			}
		}
		output_bytes += n;
	}
	else {
		while (output_bytes < text_length) {
			uint32_t byte = 0;
			br(byte, 8);
			if (byte == 0) {
				break;
			}
			if (byte >= 1 && byte <= 8) {
				for (uint8_t i = 0; i < byte; i++) {
					uint32_t b = 0;
					br(b, 8);
					output.push_back(b);
					output_bytes++;
				}
			}
			if (byte >= 0x09 && byte <= 0x7F) {
				output.push_back(byte);
				output_bytes++;
			}
			if (byte >= 0x80 && byte <= 0xBF) {
				br(byte, 8);
				assert(((byte >> 14) & 3) == 2);
				uint16_t distance = (byte >> 3) & 2047;
				assert(distance > 0 && distance < 2048);
				uint8_t length = (byte & 7);
				assert(length >= 0 && length <= 7);
				length += 3;
				size_t start = output.size() - distance;
				assert(start < output.size());
				for (uint8_t i = 0; i < length; i++) {
					output.push_back(output[start++]);
					output_bytes++;
				}
			}
			if (byte >= 0xC0 && byte <= 0xFF) {
				output.push_back(0x20);
				byte = byte & 0x7F;
				output.push_back(byte);
			}
		}
	}
	os.write(reinterpret_cast<char*>(output.data()), output.size());
}

int main(int argc, char** argv) {
	if (argc != 3) {
		return EXIT_FAILURE;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return EXIT_FAILURE;
	}
	std::ofstream os(argv[2], std::ios::binary);
	if (!os) {
		return EXIT_FAILURE;
	}

	uint8_t byte = 0xEF;
	os.put(byte);
	byte = 0xBB;
	os.put(byte);
	byte = 0xBF;
	os.put(byte);

	uint16_t num_records = 0;
	if (!read_header(is, num_records)) {
		return EXIT_FAILURE;
	}

	std::vector<uint32_t> offsets;
	for (uint16_t i = 0; i < num_records; i++) {
		uint32_t offset = 0;
		read_record_info_entry(is, i, offset);
		offsets.push_back(offset);
	}
	std::println();
	is.seekg(offsets[0]);

	uint16_t record_count = 0;
	uint32_t text_length = 0;
	if (!read_palm_header(is, record_count, text_length)) {
		return EXIT_FAILURE;
	}
	bitreader br(is);
	size_t output_bytes = 0;
	std::vector<uint8_t> output{};
	for (uint16_t i = 0; i < record_count; i++) {
		assert(i + 1 < offsets.size());
		is.seekg(offsets[i+1]);
		bool last = i + 1 == record_count;
		read_record(is, os, output_bytes, text_length, br, last);
	}
	
	return EXIT_SUCCESS;
	
}