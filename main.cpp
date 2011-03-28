#include "VPRNetParser.hpp"
#include "NetlistReader.hpp"
#include <fstream>

int main(int argc, char** argv) {
    if(argc != 2) {
        cerr << "Invalid number of arguments." << endl;
        cerr << "usage: " << argv[0] << " <benchmark path>" << endl;
        exit(-1);
    }

    ifstream file1(argv[1]);

    NetlistReader r(file1);

    r.parse();

    file1.close();
	return 0;
}
