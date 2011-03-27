#ifndef ___BLOCK_HANDLER__HPP___
#define ___BLOCK_HANDLER__HPP___

#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>
#include <cstring>
#define boolean     bool
#include "vpr_types.h"
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <boost/foreach.hpp>
#include "VPRNetParser.hpp"

using namespace std;

typedef map<string, int> net_map_t;

struct parse_data_t {
    t_subblock_data *subblock_data_ptr;
    struct s_block *block_list;
    struct s_net *net_list;
    t_subblock **subblock_list;
    int *subblocks_count;
    net_map_t net_map;
};


class BlockHandler {
public:
    int funcblock_count;
    int input_count;
    int output_count;
    int global_count;
    map<string, vector<int> > pin_class_map;
    

    BlockHandler() : global_count(0), funcblock_count(0), input_count(0), 
                    output_count(0) {
        pin_class_map[".input"] = boost::assign::list_of
                                                (RECEIVER)
                                                (DRIVER)
                                                (RECEIVER)
                                                (RECEIVER)
                                                (DRIVER)
                                                (RECEIVER)
                                                (RECEIVER)
                                                (DRIVER)
                                                (RECEIVER);
        pin_class_map[".output"] = pin_class_map[".input"];
        pin_class_map[".clb"] = boost::assign::list_of
                                                (RECEIVER)
                                                (RECEIVER)
                                                (RECEIVER)
                                                (RECEIVER)
                                                (DRIVER)
                                                (RECEIVER);
    }

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


class CountPass : public BlockHandler {
public:
    int net_count;
    CountPass() : net_count(0), BlockHandler() {}

    virtual void process_funcblock(const string &funcblocktype, 
            const string &label, const vector<string> &pins,
            const vector<SubBlock> &subblocks) {
        BlockHandler::process_funcblock(funcblocktype, label, pins, subblocks);
        for(int i = 0; i < pins.size(); i++) {
            if(pins[i] != "open") {
                int pin_class = pin_class_map[funcblocktype][i];
                if(pin_class == RECEIVER) {
                    // do nothing for now
                } else if(pin_class == DRIVER) {
                    net_count++;
                }
            }
        }
    }

    virtual void process_input(const string &label, const vector<string> &pins) {
        BlockHandler::process_input(label, pins);
        net_count++;
    }

    virtual void parse(VPRNetParser &p, ifstream &in_stream) {
        p.init();
        p.register_input_process_func(boost::bind(&CountPass::process_input, this, _1, _2));
        p.register_output_process_func(boost::bind(&CountPass::process_output, this, _1, _2));
        p.register_funcblock_process_func(boost::bind(&CountPass::process_funcblock, this, _1, _2, _3, _4));
        p.register_global_process_func(boost::bind(&CountPass::process_global, this, _1));
        p.parse(in_stream);
    }
};


class LoadPass : public BlockHandler {
    t_subblock_data *subblock_data_ptr;
    struct s_block *block_list;
    struct s_net *net_list;
    t_subblock **subblock_list;
    int *subblocks_count;
    net_map_t *p_net_map;
public:
    int net_count;
    int net_index;
    int block_count;
    int block_index;

    LoadPass(int net_count, int block_count) : 
                net_index(0),
                block_index(0), net_count(net_count), 
                block_count(block_count), BlockHandler() {
            block_list = (struct s_block *)malloc(sizeof(struct s_block) * block_count);
            memset(block_list, 0, (sizeof(struct s_block) * block_count));

            net_list = (struct s_net *)malloc(sizeof(struct s_net) * net_count);
            memset(net_list, 0, (sizeof(struct s_net) * net_count));

            subblock_list = (t_subblock **)malloc(sizeof(t_subblock *) * block_count);
            memset(subblock_list, 0, (sizeof(t_subblock *) * block_count));

            subblocks_count = (int *)malloc(sizeof(int) * block_count);
            memset(subblocks_count, 0, (sizeof(int) * block_count));
    }

    virtual void process_funcblock(const string &funcblocktype, 
            const string &label, const vector<string> &pins,
            const vector<SubBlock> &subblocks) {
        BlockHandler::process_funcblock(funcblocktype, label, pins, subblocks);
        for(int i = 0; i < pins.size(); i++) {
            if(pins[i] != "open") {
                int pin_class = pin_class_map[funcblocktype][i];
                if(pin_class == RECEIVER) {
                    // do nothing for now
                } else if(pin_class == DRIVER) {
                    process_net(pins[i]);
                    net_index++;
                }
            }
        }
        
        int num_pins = pin_class_map[funcblocktype].size();
        block_list[block_index].nets = (int *)calloc(num_pins, sizeof(int));
        subblocks_count[block_index] = subblocks.size();
        process_block(label);
        block_index++;
    }

    void process_net(const string &label) {
        net_list[net_index].name = (char *)calloc(label.size() + 1, sizeof(char));
        strcpy(net_list[net_index].name, label.c_str());
        (*p_net_map)[label] = net_index;
    }

    void process_block(const string &label) {
        block_list[block_index].name = (char *)calloc(label.size() + 1, sizeof(char));
        strcpy(block_list[block_index].name, label.c_str());
    }

    virtual void process_input(const string &label, const vector<string> &pins) {
        BlockHandler::process_input(label, pins);
        process_net(pins[0]);
        net_index++;
        int num_pins = pin_class_map[".input"].size();
        block_list[block_index].nets = (int *)calloc(num_pins, sizeof(int));
        process_block(label);
        block_index++;
    }

    virtual void process_output(const string &label, const vector<string> &pins) {
        BlockHandler::process_output(label, pins);
        int num_pins = pin_class_map[".output"].size();
        block_list[block_index].nets = (int *)calloc(num_pins, sizeof(int));
        process_block(label);
        block_index++;
    }

    void parse(VPRNetParser &p, ifstream &in_stream, parse_data_t &d) {
        p_net_map = &d.net_map;
        p.init();
        p.register_input_process_func(boost::bind(&LoadPass::process_input, this, _1, _2));
        p.register_output_process_func(boost::bind(&LoadPass::process_output, this, _1, _2));
        p.register_funcblock_process_func(boost::bind(&LoadPass::process_funcblock, this, _1, _2, _3, _4));
        p.register_global_process_func(boost::bind(&LoadPass::process_global, this, _1));
        p.parse(in_stream);

        d.subblock_data_ptr = subblock_data_ptr;
        d.block_list = block_list;
        d.net_list = net_list;
        d.subblock_list = subblock_list;
        d.subblocks_count = subblocks_count;
    }

    void display_contents() {
        for(int i = 0; i < block_index; i++) {
            cout << "block[" << i << "]: " << block_list[i].name << " has " << subblocks_count[i] << " subblocks" << endl;
        }
        for(int i = 0; i < net_index; i++) {
            cout << "net[" << i << "]: " << net_list[i].name << endl;
        }
    }
};


class MapPass : public BlockHandler {
    t_subblock_data *subblock_data_ptr;
    struct s_block *block_list;
    struct s_net *net_list;
    t_subblock **subblock_list;
    int *subblocks_count;
    net_map_t &net_map;
public:
    int net_count;
    int net_index;
    int block_count;
    int block_index;
    int total_subblocks;

    MapPass(int net_count, int block_count, parse_data_t &data) : 
                net_index(0), block_index(0), net_count(net_count),
                block_count(block_count), BlockHandler(),
                subblock_data_ptr(data.subblock_data_ptr),
                block_list(data.block_list), net_list(data.net_list),
                net_map(data.net_map),
                subblocks_count(data.subblocks_count),
                subblock_list(data.subblock_list) {
        total_subblocks = 0;
        for(int i = 0; i < block_count; i++) {
            total_subblocks += subblocks_count[i];
        }            
        subblock_list = (t_subblock **)calloc(block_count, sizeof(t_subblock*));
        memset(subblock_list, 0, sizeof(t_subblock *) * block_count);
    }

    virtual void process_funcblock(const string &funcblocktype, 
            const string &label, const vector<string> &pins,
            const vector<SubBlock> &subblocks) {
        BlockHandler::process_funcblock(funcblocktype, label, pins, subblocks);
        int num_pins = pin_class_map[funcblocktype].size();
        for(int i = 0; i < pins.size(); i++) {
            if(pins[i] != "open") {
                int pin_class = pin_class_map[funcblocktype][i];
                if(pin_class == RECEIVER) {
                    // do nothing for now
                } else if(pin_class == DRIVER) {
                    net_index++;
                }
                block_list[block_index].nets[i] = net_map[pins[i]];
            } else {
                block_list[block_index].nets[i] = OPEN;
            }
        }
        if(subblocks.size() > 0) {
            subblock_list[block_index] = (t_subblock *)calloc(subblocks_count[block_index], sizeof(t_subblock));
            for(int i = 0; i < subblocks.size(); i++) {
    /*
        max_subblocks:        1
        max_subblock_inputs:  4
        max_subblock_outputs: 1
        max_subblock_outputs: 1
        num_pins:             6
    */
                int max_subblocks = 1;
                int max_subblock_inputs = 4;
                int max_subblock_outputs = 1;

                t_subblock &sb = subblock_list[block_index][i];
                sb.name = (char*)calloc(subblocks[i].label.size() + 1, sizeof(char));
                sb.inputs = (int*)calloc(max_subblock_inputs, sizeof(int));
                sb.outputs = (int*)calloc(max_subblock_outputs, sizeof(int));

                strcpy(sb.name, subblocks[i].label.c_str());
                int j;
                for(j = 0; j < max_subblock_inputs; j++) {
                    const string &pin_label = subblocks[i].pins[j];
                    if(pin_label == "open") {
                        sb.inputs[j] = OPEN;
                    } else if(strncmp("ble_", pin_label.c_str(), 4) == 0) {
                        sb.inputs[j] = 
                            num_pins + boost::lexical_cast<int>(pin_label.c_str() + 4);
                    } else {
                        sb.inputs[j] = 
                            boost::lexical_cast<int>(pin_label);
                    }
                }
                for(j = max_subblock_inputs; j < max_subblock_inputs + max_subblock_outputs; j++) {
                    int pin_number = j - max_subblock_inputs; 
                    const string &pin_label = subblocks[i].pins[j];
                    if(pin_label == "open") {
                        sb.outputs[pin_number] = OPEN;
                    } else {
                        sb.outputs[pin_number] = boost::lexical_cast<int>(pin_label);
                    }
                }
                const string &pin_label = subblocks[i].pins[j];
                if(pin_label == "open") {
                    sb.clock = OPEN;
                } else {
                    sb.clock = boost::lexical_cast<int>(pin_label);
                }
            }
        }
        block_index++;
    }

    virtual void process_input(const string &label, const vector<string> &pins) {
        BlockHandler::process_input(label, pins);
        bool used_pin = false;

        for(int i = 0; i < pin_class_map[".input"].size(); i++) {
            if(!used_pin && pin_class_map[".input"][i] == DRIVER) {
                // We choose the first DRIVER pin as the input pin of an IO
                // pad.
                used_pin = true;
                block_list[block_index].nets[i] = net_map[pins[0]];
            } else {
                block_list[block_index].nets[i] = OPEN;
            }
        }
        net_index++;
        block_index++;
    }

    virtual void process_output(const string &label, const vector<string> &pins) {
        BlockHandler::process_output(label, pins);
        bool used_pin = false;

        for(int i = 0; i < pin_class_map[".output"].size(); i++) {
            if(!used_pin && pin_class_map[".output"][i] == RECEIVER) {
                // We choose the first RECEIVER pin as the output pin of an IO
                // pad.
                used_pin = true;
                block_list[block_index].nets[i] = net_map[pins[0]];
            } else {
                block_list[block_index].nets[i] = OPEN;
            }
        }
        block_index++;
    }

    void parse(VPRNetParser &p, ifstream &in_stream) {
        p.init();
        p.register_input_process_func(boost::bind(&MapPass::process_input, this, _1, _2));
        p.register_output_process_func(boost::bind(&MapPass::process_output, this, _1, _2));
        p.register_funcblock_process_func(boost::bind(&MapPass::process_funcblock, this, _1, _2, _3, _4));
        p.register_global_process_func(boost::bind(&MapPass::process_global, this, _1));
        p.parse(in_stream);
    }

    void display_contents() {
        for(int i = 0; i < block_index; i++) {
            cout << "block[" << i << "]: " << block_list[i].name << " has " << subblocks_count[i] << " subblocks" << endl;
            for(int j = 0; j < 3; j++) {
                cout << "  net[" << j << "]: " << block_list[i].nets[j] << endl;
            }
            for(int j = 0; j < subblocks_count[i]; j++) {
                cout << "  subblock[" << subblock_list[i][j].name << "]" << endl;
            }
        }
        for(int i = 0; i < net_index; i++) {
            cout << "net[" << i << "]: " << net_list[i].name << endl;
        }
    }
};


class VerboseHandler : public BlockHandler {
    t_subblock_data *subblock_data_ptr;
    struct s_block *block_list;
    struct s_net *net_list;
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
