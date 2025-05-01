#include <cstdint>
#include <fstream>


int main(int argc, char** argv) {
	if (argc != 2) {
		return EXIT_FAILURE;
	}
	std::ofstream os(argv[1], std::ios::binary);
	if (!os) {
		return EXIT_FAILURE;
	}
	char magic_identifier[4] = "P7\n";
	os.write(magic_identifier, 3);
	char width_string[7] = "WIDTH ";
	os.write(width_string, 6);
	char size[4] = "256";
	os.write(size, 3);
	os.put('\n');
	char height_string[8] = "HEIGHT ";
	os.write(height_string, 7);
	os.write(size, 3);
	os.put('\n');
	char depth_string[7] = "DEPTH ";
	os.write(depth_string, 6);
	char channels[2] = "1";
	os.write(channels, 1);
	os.put('\n');
	char maxval_string[8] = "MAXVAL ";
	os.write(maxval_string, 7);
	char maxval[4] = "255";
	os.write(maxval, 3);
	os.put('\n');
	char tupltype[20] = "TUPLTYPE GRAYSCALE\n";
	os.write(tupltype, 19);
	char endhdr[8] = "ENDHDR\n";
	os.write(endhdr, 7);
	for (int r = 0; r < 256; r++) {
		for (int c = 0; c < 256; c++) {
			os.put(static_cast<char>(r));
		}
	}
	return EXIT_SUCCESS;
}