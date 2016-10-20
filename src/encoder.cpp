#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

#include <dirent.h>
#include "thread.h"

static vector<string>
readDirectory(const string &directoryLocation, const string &extension);
bool Encode(const string& input);

int main(int argc, char *argv[])
{
	if (argc != 2) {
		cout << "Usage: " << argv[0] << " <input/output directory>" << '\n';
		return 1;
	}
	const string path(argv[1]);
	vector<string> files;

	try {
		files = readDirectory(path, "wav");
	}
	catch(const exception& e) {
		cout << "Error reading from " << path << ": " << e.what() << '\n';
		return 1;
	}
	ThreadPool(files, Encode);
}

bool Encode(const string& input)
{
	string output(input);
	output.replace(output.end()-3, output.end(), "mp3");
	ifstream wav;
	ofstream mp3;

	try {
		wav.open(input, ios_base::binary);
		mp3.open(output, ios_base::binary);
	}
	catch (const exception& e) {
		cout << "Error opening input/output file: " << e.what() << '\n';
		return false;
	}


    const int PCM_SIZE = 8192;
    const int MP3_SIZE = 8192;
	const int LAME_GOOD = 5;
	short int pcm_buffer[PCM_SIZE * 2];
	unsigned char mp3_buffer[MP3_SIZE];
	const size_t bytes_per_sample = 2 * sizeof(short); // stereo signal, 16 bits


    lame_t lame = lame_init();
	lame_set_in_samplerate(lame, 44100);
	lame_set_VBR(lame, vbr_default);
	lame_set_VBR_q(lame, LAME_GOOD);
	lame_init_params(lame);

	while (wav.good()) {
		int read, write;
		wav.read(reinterpret_cast<char*>(pcm_buffer), sizeof(pcm_buffer));
		read = wav.gcount() / bytes_per_sample;
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

    string lcExtension(strToLower(extension));
	
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir(directoryLocation.c_str())) == nullptr) {
        throw runtime_error("Unable to open directory.");
    }

    while ((ent = readdir(dir)) != nullptr)
    {
        string entry( ent->d_name );
        string lcEntry(strToLower(entry));
        
        // Check extension matches (case insensitive)
        size_t pos = lcEntry.rfind(lcExtension);
        if (pos != string::npos && pos == lcEntry.length() - lcExtension.length()) {
              result.push_back(directoryLocation + '/' + entry);
        }
    }

    if (closedir(dir) != 0) {
    	throw runtime_error("Unable to close directory.");
    }

    return result;
}


