#include "VPRNetParser.hpp"
#include "BlockHandler.hpp"
#include <fstream>

int main(int argc, char** argv) {
    VPRNetParser parser(32 << 10);
    BlockHandler b;

    if(argc != 2) {
        cerr << "Invalid number of arguments." << endl;
        cerr << "usage: " << argv[0] << " <benchmark path>" << endl;
        exit(-1);
    }
    ifstream file1(argv[1]);

    cout << "Initialize" << endl;
	parser.init();
    parser.register_input_process_func(boost::bind(&BlockHandler::process_input, b, _1, _2));
    parser.register_output_process_func(boost::bind(&BlockHandler::process_output, b, _1, _2));
    parser.register_clb_process_func(boost::bind(&BlockHandler::process_clb, b, _1, _2));
    parser.parse(file1);
    cout << "Done" << endl;
    file1.close();
	return 0;
}
