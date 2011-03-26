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

	parser.init();
    // Here we register the functions of the BlockHandler instance b to handle
    // the processing of function blocks, inputs, and outputs, using the Boost bind
    // library.  Notices that we pass the address of b (i.e., &b), since if we
    // simply passed "b" to the bind function, a copy would be made of b and
    // any changes state updates would not be made to instance b.
    parser.register_input_process_func(boost::bind(&BlockHandler::process_input, &b, _1, _2));
    parser.register_output_process_func(boost::bind(&BlockHandler::process_output, &b, _1, _2));
    parser.register_funcblock_process_func(boost::bind(&BlockHandler::process_funcblock, &b, _1, _2, _3, _4));
    parser.register_global_process_func(boost::bind(&BlockHandler::process_global, &b, _1));
    parser.parse(file1);


    cout << "------------------------------------------------------------------------" << endl;
    cout << "PASS 1:" << endl;
    // If we had simply passed "b" to bind above, all of the following counts
    // would be zero, since the state of instance "b" would remain unchanged.
    cout << "  Funcblock count:    " << b.funcblock_count << endl;
    cout << "  Input count:        " << b.input_count << endl;
    cout << "  Output count:       " << b.output_count << endl;
    cout << "  Global count:       " << b.global_count << endl;

    cout << "------------------------------------------------------------------------" << endl;
    cout << "PASS 2:" << endl;

    file1.clear();
    file1.seekg(0, ios::beg);

    VerboseHandler v;
    parser.register_input_process_func(boost::bind(&VerboseHandler::process_input, &v, _1, _2));
    parser.register_output_process_func(boost::bind(&VerboseHandler::process_output, &v, _1, _2));
    parser.register_funcblock_process_func(boost::bind(&VerboseHandler::process_funcblock, &v, _1, _2, _3, _4));
    parser.register_global_process_func(boost::bind(&VerboseHandler::process_global, &v, _1));
    parser.parse(file1);

    file1.close();
	return 0;
}
