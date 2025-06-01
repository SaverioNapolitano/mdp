#include <fstream>
#include <vector>
#include <cstdint>
#include <string_view>
#include <ranges>
#include <unordered_map>
#include <cmath>
#include <print>
#include <numbers>
#include <algorithm>

// write auto in a parameter = template function 
// to get the type -> decltype(<variable>) 
// concept: types of types (meta-type) -> features that a data type have -> when using templates allow to specify requirements and to catch errors at compile-time
std::ostream& write(std::ostream& os, const auto& val) {
    return os.write(reinterpret_cast<const char*>(&val), sizeof(val));
}
std::ostream& write(std::ostream& os, const auto& val, size_t size) {
    return os.write(reinterpret_cast<const char*>(&val), size);
}
std::ostream& write(std::ostream& os, const auto* val, size_t size) {
    return os.write(reinterpret_cast<const char*>(val), size);
}
std::ostream& write(std::ostream& os, const char* val) {
    return os.write(val, strlen(val));
}

std::vector<int16_t> load_samples(std::string_view filename)
{
    std::vector<int16_t> samples;
    std::ifstream is(filename.data(), std::ios::binary);
    if (is) {
        is.seekg(0, std::ios::end);
        size_t size = is.tellg();
        is.seekg(0, std::ios::beg);
        samples.resize(size / 2);
        is.read(reinterpret_cast<char*>(samples.data()), size);
    }
    return samples;
}

void save_wav(std::string_view filename, const std::vector<int16_t>& samples)
{
    std::ofstream os(filename.data(), std::ios::binary);
    if (os) {
        const uint32_t sampleRate = 44100;
        const uint16_t numChannels = 1;
        const uint16_t bitsPerSample = 16;

        uint32_t numSamples = static_cast<uint32_t>(size(samples));
        const uint32_t byteRate = sampleRate * numChannels * bitsPerSample / 8;
        const uint16_t blockAlign = numChannels * bitsPerSample / 8;
        const uint32_t dataSize = static_cast<uint32_t>(numSamples * blockAlign);
        const uint32_t chunkSize = 36 + dataSize;

        // Header RIFF
        write(os, "RIFF");
        write(os, chunkSize);
        write(os, "WAVE");
        // Subchunk1 "fmt "
        write(os, "fmt ");
        uint32_t subchunk1Size = 16;
        uint16_t audioFormat = 1;
        write(os, subchunk1Size);
        write(os, audioFormat);
        write(os, numChannels);
        write(os, sampleRate);
        write(os, byteRate);
        write(os, blockAlign);
        write(os, bitsPerSample);
        // Subchunk2 "data"
        write(os, "data");
        write(os, dataSize);
        write(os, samples.data(), size(samples) * 2);
    }
}

template<typename T>
struct frequency {
    std::unordered_map<T, size_t> count_;
    frequency(const std::ranges::forward_range auto& seq) {
        for (const auto& x : seq) {
            ++count_[x];
        }
    }
    auto begin(this auto&& self) { return std::begin(self.count_); }
    auto end(this auto&& self) { return std::end(self.count_); }
    auto size(this auto&& self) { return std::size(self.count_); }
};

double entropy(const std::ranges::forward_range auto& seq)
{
    frequency<std::ranges::range_value_t<decltype(seq)>> freq(seq);
    double N = 0.0;
    double s = 0.0;
    for (const auto& [x, f] : freq) {
        s += f * log2(f);
        N += f;
    }
    double H = log2(N) - s / N;
    return H;
}

struct MDCT {
    std::vector<std::vector<double>> cos_;
    std::vector<double> w_;
    MDCT() : cos_(2048, std::vector<double>(1024)), w_(2048) {
        for (size_t n = 0; n < 2048; ++n) {
            for (size_t k = 0; k < 1024; ++k) {
                cos_[n][k] = cos(std::numbers::pi / 1024.0 *
                    (n + 0.5 + 1024.0 / 2) * (k + 0.5));
            }
            w_[n] = sin(std::numbers::pi / 2048.0 * (n + 0.5));
        }
    }

    void blockMDCT(const int16_t* x, double* X) {
        for (size_t k = 0; k < 1024; ++k) {
            double s = 0.0;
            for (size_t n = 0; n < 2048; ++n) {
                s += x[n] * w_[n] * cos_[n][k];
            }
            X[k] = s;
        }
    }

    void blockIMDCT(const double* X, double* y) {
        for (size_t n = 0; n < 2048; ++n) {
            double s = 0.0;
            for (size_t k = 0; k < 1024; ++k) {
                s += X[k] * cos_[n][k];
            }
            y[n] = 2.0 / 1024.0 * w_[n] * s;
        }
    }

    std::vector<double> forward(const std::vector<int16_t>& samples) {
        size_t nwin = 2 + (samples.size() + 1023) / 1024;
        std::vector<int16_t> padded(nwin * 1024);
        // instead of <iterator> + value, to be more generic use std::next(<iterator>, <value>)
        std::ranges::copy(samples, begin(padded) + 1024);
        std::vector<double> X(nwin * 1024);
        for (size_t i = 0; i < nwin - 1; ++i) {
            blockMDCT(padded.data() + i * 1024, X.data() + i * 1024);
        }
        return X;
    }

    std::vector<int16_t> inverse(const std::vector<double>& X) {
        size_t nwin = X.size() / 1024;
        std::vector<int16_t> y(nwin * 1024);
        double tmp[2048];
        for (size_t i = 0; i < nwin - 1; ++i) {
            blockIMDCT(X.data() + i * 1024, tmp);
            std::transform(
                tmp,
                tmp + 2048,
                y.data() + i * 1024,
                y.data() + i * 1024,
                [](double a, int16_t b) {
                    return static_cast<int16_t>(round(a + b));
                }
            );
        }
        return y;
    }
};

template<typename T>
T saturate_cast(int val) {
    if (val < std::numeric_limits<T>::min()) {
        return std::numeric_limits<T>::min();
    }
    else if (val > std::numeric_limits<T>::max()) {
        return std::numeric_limits<T>::max();
    }
    else {
        return val;
    }
}



int main(void)
{
    std::vector<int16_t> samples = load_samples("test.raw");
    std::println("Number of samples: {}", samples.size());
    std::println("Entropy of samples: {}", entropy(samples));
    int16_t Q = 2600;
    // <range> | (pipe) <lazy_view> 
    // operations in C++ are never done between int16_t and int8_t but are promoted to int32_t, we need to specify the return value in lambda function 
    auto quantized = samples
        | std::views::transform([Q](int16_t x) -> int16_t { return static_cast<int16_t>(round(double(x) / Q)); })
        | std::ranges::to<std::vector>();
    std::println("Entropy of quantized samples: {}", entropy(quantized));
    auto dequant = quantized
        | std::views::transform([Q](int16_t x) { return saturate_cast<int16_t>(x * Q); })
        | std::ranges::to<std::vector>();
    save_wav("output_qt.wav", dequant);
    auto error_qt = std::views::zip(samples, dequant)
        | std::views::transform([](auto pair) -> int16_t { return std::get<0>(pair) - std::get<1>(pair); })
        | std::ranges::to<std::vector>();
    save_wav("error_qt.wav", error_qt);

    MDCT mdct;
    std::print("Forward MDCT...");
    auto X = mdct.forward(samples);
    std::println(" done.");
    double mdctQ = 40000.0;
    auto Xquant = X
        | std::views::transform([mdctQ](double x) { return round(x / mdctQ); })
        | std::ranges::to<std::vector>();
    std::println("Entropy of quantized coefficients: {}", entropy(Xquant));
    auto Xdequant = Xquant
        | std::views::transform([mdctQ](double x) { return x * mdctQ; })
        | std::ranges::to<std::vector>();
    std::print("Inverse MDCT...");
    auto y = mdct.inverse(Xdequant);
    std::println(" done.");
    y.erase(begin(y), std::next(begin(y), 1024));
    y.resize(size(samples));
    save_wav("output.wav", y);

    auto error = std::views::zip(samples, y)
        | std::views::transform([](auto pair) -> int16_t { return std::get<0>(pair) - std::get<1>(pair); })
        | std::ranges::to<std::vector>();
    save_wav("error.wav", error);
}