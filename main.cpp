#include "VPRNetParser.hpp"
#include "BlockHandler.hpp"

int main() {
    VPRNetParser parser(32 << 10);
    BlockHandler b;
    cout << "Initialize" << endl;

	parser.init();
    parser.register_input_process_func(boost::bind(&BlockHandler::process_input, b, _1, _2));
    parser.register_output_process_func(boost::bind(&BlockHandler::process_output, b, _1, _2));
    parser.register_clb_process_func(boost::bind(&BlockHandler::process_clb, b, _1, _2));
    parser.parse();
    cout << "Done" << endl;
	return 0;
}
