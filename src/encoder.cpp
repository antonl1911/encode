#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <string>
#include <sys/stat.h>
#include <dirent.h>
#include <lame/lame.h>

#include "threadpool.h"
using std::cout;
using std::vector;
using std::exception;
using std::string;
using std::thread;

static vector<string>
readDirectory(const string &directoryLocation, const string &extension);
bool LameEncode(const string& input);

int main(int argc, char *argv[])
{
    cout << "Using LAME version " << get_lame_version() << '\n';
    auto usage = [](const char *name) {
        cout << "Usage: " << name << " <input/output directory>" << '\n';
    };
    if (argc != 2) {
        usage(argv[0]);
        return 1;
    }
    const string path(argv[1]);
    vector<string> files;

    try {
        files = readDirectory(path, ".wav");
    }
    catch(const exception& e) {
        cout << "Error reading from " << path << ": " << e.what() << '\n';
        usage(argv[0]);
        return 1;
    }
    if (files.size()) {
        try {
            ThreadPool(files, LameEncode);
        }
        catch (std::runtime_error &e) {
            cout << e.what();
            exit(1);
        }
    }
    else {
        cout << "Error: no suitable WAV files found in " << path << '\n';
        usage(argv[0]);
        return 1;
    }
}

bool LameEncode(const string& input)
{
    const size_t IN_SAMPLERATE = 44100; // default sample-rate
    const size_t PCM_SIZE = 8192;
    const size_t MP3_SIZE = 8192;
    const size_t LAME_GOOD = 5;
    int16_t pcm_buffer[PCM_SIZE * 2];
    unsigned char mp3_buffer[MP3_SIZE];
    const size_t bytes_per_sample = 2 * sizeof(int16_t); // stereo signal, 16 bits
    const string ext = {"mp3"};

    string output(input);
    output.replace(output.end() - ext.length(), output.end(), ext);
    std::ifstream wav;
    std::ofstream mp3;

    wav.exceptions(std::ifstream::failbit);
    mp3.exceptions(std::ifstream::failbit);
    try {
        wav.open(input, std::ios_base::binary);
        mp3.open(output, std::ios_base::binary);
    }
    catch (std::ifstream::failure e) {
        cout << "Error opening input/output file: " << std::strerror(errno) << '\n';
        return false;
    }

    lame_t lame = lame_init();
    lame_set_in_samplerate(lame, IN_SAMPLERATE);
    lame_set_VBR(lame, vbr_default);
    lame_set_VBR_q(lame, LAME_GOOD);

    if (lame_init_params(lame) < 0) {
        wav.close();
        mp3.close();
        return false;
    }

    while (wav.good()) {
        int write = 0;
        wav.read(reinterpret_cast<char*>(pcm_buffer), sizeof(pcm_buffer));
        int read = wav.gcount() / bytes_per_sample;
        if (read == 0)
            write = lame_encode_flush(lame, mp3_buffer, MP3_SIZE);
        else
            write = lame_encode_buffer_interleaved(lame, pcm_buffer, read, mp3_buffer, MP3_SIZE);
        mp3.write(reinterpret_cast<char*>(mp3_buffer), write);
    }

    wav.close();
    mp3.close();

    lame_close(lame);
    return true;
}

/**
 * Read a directory listing into a vector of strings, filtered by file extension.
 * Throws std::exception on error.
 **/
static vector<string> readDirectory(const string &directoryLocation, const string &extension)
{
    vector<string> result;

    auto strToLower = [](const string& input) {
        string result(input.length(), 0);
        transform(input.begin(), input.end(), result.begin(), ::tolower);
        return result;
    };
    auto GetFileSize = [](const string& filename) {
        struct stat stat_buf;
        int rc = stat(filename.c_str(), &stat_buf);
        return rc == 0 ? stat_buf.st_size : -1;
    };
    string lcExtension(strToLower(extension));

    auto *dir = opendir(directoryLocation.c_str());
    if (!dir)
        throw std::runtime_error("Unable to open directory.");

    struct dirent *ent;
    while ((ent = readdir(dir)) != nullptr)
    {
        string entry(ent->d_name);
        string lcEntry(strToLower(entry));

        // Check extension matches (case insensitive)
        auto pos = lcEntry.rfind(lcExtension);
        if (pos != string::npos && pos == lcEntry.length() - lcExtension.length()) {
            auto path = directoryLocation + '/' + entry;
            if (GetFileSize(path) > 0)
                result.push_back(path);
            else
                cout << "Truncated wav file: " << path << '\n';
        }
    }

    if (closedir(dir) != 0)
        throw std::runtime_error("Unable to close directory.");

    return result;
}


