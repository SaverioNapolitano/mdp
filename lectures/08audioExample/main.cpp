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
	// samp. freq. = 44100 Hz 
	// bit per sample = 16 bit 

	double duration = 3; // 3 s 
	double f = 440; // Hz - note: A
	double f2 = 523; // Hz - note: C next octave
	double A = 32767 / 2.0; // relative to max (32767) -> 2^15 - 1, halved to be sure we don't go out of bounds
	std::numbers::pi;

	std::vector<int16_t> samples(static_cast<size_t>(duration * 44100));

	for (size_t i = 0; i < samples.size(); i++) {
		double note1 = round(A * sin(2 * std::numbers::pi * f * i / 44100));
		double note2 = round(A * sin(2 * std::numbers::pi * f2 * i / 44100));
		// mixing = add things together (play sounds at the same time)
		samples[i] = static_cast<int16_t>(note1 + note2);
		// we can hear two separate sounds / frequencies (unlike our eyes which can't perform frequency analysis, e.g. we see one color and not its components)
	}

	write_samples("audio1.raw", samples);

	std::ifstream is("test.raw", std::ios::binary);
	is.seekg(0, std::ios::end);
	size_t size = is.tellg();
	is.seekg(0, std::ios::beg);
	std::vector<int16_t> audio(size / 2);
	is.read(reinterpret_cast<char*>(audio.data()), size);
	for (auto& x : audio) {
		//x = saturate(x * 2);
		x = x / 1000 * 1000;
	}

	write_samples("audio2.raw", audio);




}