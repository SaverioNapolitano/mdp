#include <fstream>
#include <cstdint>
#include <vector>
#include <exception>
#include <array>
#include <algorithm>
#include <cmath>
#include <string>
#include <iostream>
#include <bit>
#include <cassert>

using qoa_slice_t = uint64_t;

void save_wav(std::ostream& os, std::vector<int16_t>& samples) {
	std::string riff("RIFF");
	os.write(riff.data(), 4);
	uint32_t size = samples.size() * 2 + 44 - 8;
	os.write(reinterpret_cast<char*>(&size), 4);
	std::string wave("WAVE");
	os.write(wave.data(), 4);
	std::string fmt("fmt ");
	os.write(fmt.data(), 4);
	uint32_t len = 16;
	os.write(reinterpret_cast<char*>(&len), 4);
	uint16_t fmt_type = 1;
	os.write(reinterpret_cast<char*>(&fmt_type), 2);
	uint16_t num_channels = 2;
	os.write(reinterpret_cast<char*>(&num_channels), 2);
	uint32_t samplerate = 44100;
	os.write(reinterpret_cast<char*>(&samplerate), 4);
	uint32_t n = 176400;
	os.write(reinterpret_cast<char*>(&n), 4);
	uint16_t bpsc = 4;
	os.write(reinterpret_cast<char*>(&bpsc), 2);
	uint16_t bps = 16;
	os.write(reinterpret_cast<char*>(&bps), 2);
	std::string data("data");
	os.write(data.data(), 4);
	size_t rawsize = samples.size() * sizeof(int16_t);
	os.write(reinterpret_cast<char*>(&rawsize), 4);
	
	for (size_t i = 0, j = 20; i < samples.size(); i += 40, j += 40) {
		size_t l = 0, r = 0;
		size_t a = 20, b = 20;
		if (samples.size() - i < 40) {
			a = b = (samples.size() - i) / 2;
			j -= (20 - b);
		}
		while (l < a && r < b) {
			if (i + l < samples.size()) {
				os.write(reinterpret_cast<char*>(&samples[i + l]), 2);
			}
			if (j + r < samples.size()) {
				os.write(reinterpret_cast<char*>(&samples[j + r]), 2);
			}
			l++;
			r++;
		}
	}
}

struct qoa_file_t {
	struct file_header {
		std::string magic_;
		uint32_t samples_;
	};

	struct frame {
		struct frame_header {
			uint8_t num_channels_;
			uint32_t samplerate_ = 0; // 24 bits
			uint16_t fsamples_;
			uint16_t fsize_;
		};

		struct lms_state {
			std::array<int16_t,4> history;
			std::array<int16_t, 4> weights;
		};
		frame_header fh_;
		std::vector<lms_state> states_;
	};

	std::istream& is_;
	file_header file_head_;
	std::vector<frame> frames_;
	std::array<double, 8> dequant_tab_{0.75, -0.75, 2.5, -2.5, 4.5, -4.5, 7, -7};
	std::vector<int16_t> rec_;

	qoa_file_t(std::istream& is):is_(is){
		read_file_header();
		for (size_t i = 0; i < frames_.size(); i++) {
			read_frame(i);
		}
	}

	void read_file_header() {
		using namespace std::string_literals;
		file_head_.magic_.resize(4);
		is_.read(file_head_.magic_.data(), 4);
		if (file_head_.magic_ != "qoaf"s) {
			exit(EXIT_FAILURE);
		}
		is_.read(reinterpret_cast<char*>(&(file_head_.samples_)), 4);
		file_head_.samples_ = std::byteswap(file_head_.samples_);
		frames_.resize(std::ceil(file_head_.samples_ / (256.0 * 20.0)));

	}

	void read_frame_header(size_t i) {
		frame f = frames_[i];
		f.fh_.num_channels_ = is_.get();
		f.states_.resize(f.fh_.num_channels_);
		if (f.fh_.num_channels_ != 2) {
			exit(EXIT_FAILURE);
		}
		uint8_t b1 = is_.get();
		uint8_t b2 = is_.get();
		uint8_t b3 = is_.get();
		f.fh_.samplerate_ = b1;
		f.fh_.samplerate_ = (f.fh_.samplerate_ << 8) | b2;
		f.fh_.samplerate_ = (f.fh_.samplerate_ << 8) | b3;
		if (f.fh_.samplerate_ != 44100) {
			exit(EXIT_FAILURE);
		}
		is_.read(reinterpret_cast<char*>(&(f.fh_.fsamples_)), 2);
		f.fh_.fsamples_ = std::byteswap(f.fh_.fsamples_);
		is_.read(reinterpret_cast<char*>(&(f.fh_.fsize_)), 2);
		f.fh_.fsize_ = std::byteswap(f.fh_.fsize_);
		frames_[i] = f;
	}

	void read_state(size_t i, size_t l) {
		is_.read(reinterpret_cast<char*>(frames_[i].states_[l].history.data()), 8);
		is_.read(reinterpret_cast<char*>(frames_[i].states_[l].weights.data()), 8);
		
		for (auto& h : frames_[i].states_[l].history) {
			h = std::byteswap(h);
		}
		
		for (auto& w : frames_[i].states_[l].weights) {
			w = std::byteswap(w);
		}
		
		
		
	}

	void read_frame(size_t i) {
		read_frame_header(i);
		for (size_t l = 0; l < frames_[i].fh_.num_channels_; l++) {
			read_state(i, l);
		}
		uint16_t bytes = 0;
		uint16_t size = frames_[i].fh_.fsize_ - frames_[i].states_.size() * 16 - 8;
		if (i == frames_.size() - 1) {
			uint16_t full_slices = frames_[i].fh_.fsamples_ / 20;
			std::vector<uint16_t> slices(frames_[i].fh_.num_channels_);
			std::vector<uint16_t> samples(frames_[i].fh_.num_channels_);
			uint8_t counter = 0;
			while (counter < frames_[i].fh_.num_channels_) {
				for (size_t k = 0; k < frames_[i].fh_.num_channels_; k++) {
					if (slices[k] > full_slices) {
						counter++;
						break;
					}
					qoa_slice_t slice;
					is_.read(reinterpret_cast<char*>(&slice), sizeof(qoa_slice_t));
					slice = std::byteswap(slice);
					slices[k]++;
					uint8_t sf_quant = (slice >> 60) & 0xF;
					double sf = std::round(std::pow(sf_quant + 1, 2.75));
					for (size_t j = 3; j <= 60; j += 3) {
						if (samples[k] < frames_[i].fh_.fsamples_) {
							uint8_t qr = (slice >> (60 - j)) & 7;
							double r = sf * dequant_tab_[qr];
							r = r < 0 ? std::ceil(r - 0.5) : std::floor(r + 0.5);
							int64_t p = 0;
							for (size_t n = 0; n < 4; n++) {
								p += frames_[i].states_[k].history[n] * frames_[i].states_[k].weights[n];
							}
							p >>= 13;
							int16_t s = static_cast<int16_t>(std::clamp(static_cast<int32_t>(p + r), -32768, 32767));

							rec_.push_back(s);

							int16_t delta = static_cast<int32_t>(r) >> 4;
							for (size_t n = 0; n < 4; n++) {
								frames_[i].states_[k].weights[n] += frames_[i].states_[k].history[n] < 0 ? -1 * delta : delta;
							}
							for (size_t n = 0; n < 3; n++) {
								frames_[i].states_[k].history[n] = frames_[i].states_[k].history[n + 1];
							}
							frames_[i].states_[k].history[3] = s;
							samples[k]++;
						}
						else {
							break;
						}
					}
				}
			}
			
		}
		else {
			while (bytes < size) {
				for (size_t k = 0; k < frames_[i].fh_.num_channels_; k++) {
					qoa_slice_t slice;
					is_.read(reinterpret_cast<char*>(&slice), sizeof(qoa_slice_t));
					slice = std::byteswap(slice);
					bytes += 8;
					uint8_t sf_quant = (slice >> 60) & 0xF;
					double sf = std::round(std::pow(sf_quant + 1, 2.75));
					for (size_t j = 3; j <= 60; j += 3) {
						uint8_t qr = (slice >> (60 - j)) & 7;
						double r = sf * dequant_tab_[qr];
						r = r < 0 ? std::ceil(r - 0.5) : std::floor(r + 0.5);
						int32_t p = 0;
						for (size_t n = 0; n < 4; n++) {
							p += frames_[i].states_[k].history[n] * frames_[i].states_[k].weights[n];
						}
						p >>= 13;
						int16_t s = static_cast<int16_t>(std::clamp(static_cast<int32_t>(p + r), -32768, 32767));

						rec_.push_back(s);

						int16_t delta = static_cast<int16_t>(r) >> 4;
						for (size_t n = 0; n < 4; n++) {
							frames_[i].states_[k].weights[n] += frames_[i].states_[k].history[n] < 0 ? -1 * delta : delta;
						}
						for (size_t n = 0; n < 3; n++) {
							frames_[i].states_[k].history[n] = frames_[i].states_[k].history[n + 1];
						}
						frames_[i].states_[k].history[3] = s;
					}
				}
			}
		}
		
		
	}
			
};


int main(int argc, char** argv) {
	if (argc != 3) {
		return EXIT_FAILURE;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return EXIT_FAILURE;
	}

	qoa_file_t qoa(is);
	std::ofstream os(argv[2], std::ios::binary);
	if (!os) {
		return EXIT_FAILURE;
	}
	save_wav(os, qoa.rec_);

	return EXIT_SUCCESS;
}