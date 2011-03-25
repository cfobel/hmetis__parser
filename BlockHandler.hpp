#ifndef ___BLOCK_HANDLER__HPP___
#define ___BLOCK_HANDLER__HPP___

#include <string>
#include <vector>
#include <boost/foreach.hpp>

class BlockHandler {
public:
    int clb_count;
    int input_count;
    int output_count;

    BlockHandler() : clb_count(0), input_count(0), output_count(0) {}
    void process_clb(const string &label, const vector<string> &pins,
            const vector< vector<string> > &subblocks) {
        clb_count++;
        cout << "CLB: " << label << endl;
        cout << "  Pins: ";
        for(int i = 0; i < pins.size(); i++) {
            cout << " " << pins[i];
        }
        cout << endl;
        int i = 0;
        BOOST_FOREACH(const vector<string> &v, subblocks) {
            cout << "  Subblock[" << i++ << "]:";
            BOOST_FOREACH(string s, v) {
                cout << " " << s;
            }
            cout << endl;
        }
    }

    void process_input(const string &label, const vector<string> &pins) {
        input_count++;
        cout << "input: " << label << endl;
        cout << "  Pins: ";
        for(int i = 0; i < pins.size(); i++) {
            cout << " " << pins[i];
        }
        cout << endl;
    }

    void process_output(const string &label, const vector<string> &pins) {
        output_count++;
        cout << "output: " << label << endl;
        cout << "  Pins: ";
        for(int i = 0; i < pins.size(); i++) {
            cout << " " << pins[i];
        }
        cout << endl;
    }
};

#endif
