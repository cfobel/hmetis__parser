#ifndef ___BLOCK_HANDLER__HPP___
#define ___BLOCK_HANDLER__HPP___

#include <string>
#include <vector>
#include <boost/foreach.hpp>

#define VERBOSE

class BlockHandler {
public:
    int funcblock_count;
    int input_count;
    int output_count;
    int global_count;

    BlockHandler() : global_count(0), funcblock_count(0), input_count(0), output_count(0) {}
    void process_funcblock(const string &funcblocktype, 
            const string &label, const vector<string> &pins,
            const vector<SubBlock> &subblocks) {
        funcblock_count++;
#ifdef VERBOSE
        cout << funcblocktype << ": " << label << endl;
        cout << "  Pins: ";
        for(int i = 0; i < pins.size(); i++) {
            cout << " " << pins[i];
        }
        cout << endl;
        int i = 0;
        BOOST_FOREACH(const SubBlock &subblock, subblocks) {
            cout << "  Subblock[" << subblock.label << "]:";
            BOOST_FOREACH(const string s, subblock.pins) {
                cout << " " << s;
            }
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

    void process_global(const string &label) {
        global_count++;
#ifdef VERBOSE
        cout << "global: " << label << endl;
#endif
    }
};

#endif
