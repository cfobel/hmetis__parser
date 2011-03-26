#ifndef ___BLOCK_HANDLER__HPP___
#define ___BLOCK_HANDLER__HPP___

#include <string>
#include <vector>
#include <boost/foreach.hpp>

#define VERBOSE

class BlockHandler {
public:
    int clb_count;
    int input_count;
    int output_count;

    BlockHandler() : clb_count(0), input_count(0), output_count(0) {}
    void process_clb(const string &label, const vector<string> &pins,
            const vector<SubBlock> &subblocks) {
        clb_count++;
#ifdef VERBOSE
        cout << "CLB: " << label << endl;
        cout << "  Pins: ";
        for(int i = 0; i < pins.size(); i++) {
            cout << " " << pins[i];
        }
        cout << endl;
        int i = 0;
        BOOST_FOREACH(const SubBlock &subblock, subblocks) {
            cout << "  Subblock[" << subblock.label << "]:";
            BOOST_FOREACH(const string s, subblock.input_pins) {
                cout << " " << s;
            }
            cout << " " << subblock.output_pin;
            cout << " " << subblock.clock_pin;
            cout << endl;
        }
#endif
    }

    void process_input(const string &label, const vector<string> &pins) {
        input_count++;
#ifdef VERBOSE
        cout << "input: " << label << endl;
        cout << "  Pins: ";
        for(int i = 0; i < pins.size(); i++) {
            cout << " " << pins[i];
        }
        cout << endl;
#endif
    }

    void process_output(const string &label, const vector<string> &pins) {
        output_count++;
#ifdef VERBOSE
        cout << "output: " << label << endl;
        cout << "  Pins: ";
        for(int i = 0; i < pins.size(); i++) {
            cout << " " << pins[i];
        }
        cout << endl;
#endif
    }
};

#endif
