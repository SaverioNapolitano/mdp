#include <numbers>
#include <vector>
#include <cmath>
#include <fstream>

int16_t saturate(int32_t value) {
	if (value > 32767) {
		return 32767;
	}
	if (value < -32767) {
		return -32767;
	}
	return value;
}

void write_samples(const std::string& filename, const std::vector<int16_t>& samples) {
	std::ofstream os(filename, std::ios::binary);
	os.write(reinterpret_cast<const char*>(samples.data()), samples.size() * 2);
}

int main() {
	// y = A*sin(2*pi*f*t)
	// freq. camp. = 44100 Hz 
	// bit per sample = 16 bit 

	double duration = 3; // 3 s 
	double f = 440; // Hz
	double A = 32767; // relative to max (32767) -> 2^15 - 1 
	std::numbers::pi;

	std::vector<int16_t> samples(static_cast<size_t>(duration * 44100));

	for (size_t i = 0; i < samples.size(); i++) {
		samples[i] = static_cast<int16_t>(round(A * sin(2 * std::numbers::pi * f * i / 44100)));
	}

	write_samples("audio1.raw", samples);

	std::ifstream is("test.raw", std::ios::binary);
	is.seekg(0, std::ios::end);
	size_t size = is.tellg();
	is.seekg(0, std::ios::beg);
	std::vector<int16_t> audio(size / 2);
	is.read(reinterpret_cast<char*>(audio.data()), size);
	// quantizzare mi permette di risparmiare bit nella rappresentazione ma introduce rumore 
	// possiamo quantizzare in maniera diversa in base alla percezione umana del suono 
	for (auto& x : audio) {
		//x = saturate(x * 2);
		x = x / 1000 * 1000;
	}

	write_samples("audio2.raw", audio);




}