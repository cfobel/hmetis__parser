#ifndef ___BLOCK_HANDLER__HPP___
#define ___BLOCK_HANDLER__HPP___

#include <string>
#include <vector>
#include <fstream>
#include <boost/foreach.hpp>
#include "VPRNetParser.hpp"

class BlockHandler {
public:
    int funcblock_count;
    int input_count;
    int output_count;
    int global_count;

    BlockHandler() : global_count(0), funcblock_count(0), input_count(0), output_count(0) {}
    virtual void process_funcblock(const string &funcblocktype, 
            const string &label, const vector<string> &pins,
            const vector<SubBlock> &subblocks) {
        funcblock_count++;
    }

    virtual void process_input(const string &label, const vector<string> &pins) {
        input_count++;
    }

    virtual void process_output(const string &label, const vector<string> &pins) {
        output_count++;
    }

    virtual void process_global(const string &label) {
        global_count++;
    }

    virtual void parse(VPRNetParser &p, ifstream &in_stream) {
        p.init();
        p.register_input_process_func(boost::bind(&BlockHandler::process_input, this, _1, _2));
        p.register_output_process_func(boost::bind(&BlockHandler::process_output, this, _1, _2));
        p.register_funcblock_process_func(boost::bind(&BlockHandler::process_funcblock, this, _1, _2, _3, _4));
        p.register_global_process_func(boost::bind(&BlockHandler::process_global, this, _1));
        p.parse(in_stream);
    }
};


class VerboseHandler : public BlockHandler {
public:
    VerboseHandler() : BlockHandler() {}

    virtual void process_funcblock(const string &funcblocktype, 
            const string &label, const vector<string> &pins,
            const vector<SubBlock> &subblocks) {
        BlockHandler::process_funcblock(funcblocktype, label, pins, subblocks);
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
    }

    virtual void process_input(const string &label, const vector<string> &pins) {
        BlockHandler::process_input(label, pins);
        cout << "input: " << label << endl;
        cout << "  Pins: ";
        for(int i = 0; i < pins.size(); i++) {
            cout << " " << pins[i];
        }
        cout << endl;
    }

    virtual void process_output(const string &label, const vector<string> &pins) {
        BlockHandler::process_output(label, pins);
        cout << "output: " << label << endl;
        cout << "  Pins: ";
        for(int i = 0; i < pins.size(); i++) {
            cout << " " << pins[i];
        }
        cout << endl;
    }

    virtual void process_global(const string &label) {
        BlockHandler::process_global(label);
        cout << "global: " << label << endl;
    }

    virtual void parse(VPRNetParser &p, ifstream &in_stream) {
        p.init();
        p.register_input_process_func(boost::bind(&VerboseHandler::process_input, this, _1, _2));
        p.register_output_process_func(boost::bind(&VerboseHandler::process_output, this, _1, _2));
        p.register_funcblock_process_func(boost::bind(&VerboseHandler::process_funcblock, this, _1, _2, _3, _4));
        p.register_global_process_func(boost::bind(&VerboseHandler::process_global, this, _1));
        p.parse(in_stream);
    }
};

#endif
