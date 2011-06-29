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
};


class NetlistReader {
    VPRNetParser p;
    ifstream &in_stream;
    map<string, vector<int> > pin_class_map;
    net_map_t net_map;

    int funcblock_count;
    int input_count;
    int output_count;
    int global_count;

    int net_count;
    int block_count;

    int net_index;
    int block_index;

    t_subblock_data *subblock_data_ptr;
    struct s_block *block_list;
    struct s_net *net_list;
    t_subblock **subblock_list;
    int *subblocks_count;

public:
    NetlistReader(ifstream &in_stream) : in_stream(in_stream),
            net_index(0), block_index(0),
            funcblock_count(0), input_count(0),
            output_count(0), global_count(0) {
        init_pin_class_map();
        p = VPRNetParser(32 << 10);
    }

    void reset() {
        net_index = 0;
        block_index = 0;
    }

    void init_pin_class_map() {
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

    void rewind() {
        reset();
        in_stream.clear();
        in_stream.seekg(0, ios::beg);
    }

    /* Count pass: START */
    void count_pass() {
        p.init();
        p.register_input_process_func(boost::bind(&NetlistReader::count_process_input, this, _1, _2));
        p.register_output_process_func(boost::bind(&NetlistReader::count_process_output, this, _1, _2));
        p.register_funcblock_process_func(boost::bind(&NetlistReader::count_process_funcblock, this, _1, _2, _3, _4));
        p.register_global_process_func(boost::bind(&NetlistReader::count_process_global, this, _1));
        p.parse(in_stream);
        net_count = net_index;
        block_count = block_index;
    }

    void count_process_funcblock(const string &funcblocktype, 
            const string &label, const vector<string> &pins,
            const vector<SubBlock> &subblocks) {
        funcblock_count++;
        for(int i = 0; i < pins.size(); i++) {
            const int &pin_class = pin_class_map[funcblocktype][i];
            if(pins[i] != "open" && pin_class == DRIVER) {
                net_index++;
            }
        }
        block_index++;
    }

    void count_process_input(const string &label, const vector<string> &pins) {
        input_count++;
        net_index++;
        block_index++;
    }

    void count_process_output(const string &label, const vector<string> &pins) {
        output_count++;
        block_index++;
    }

    void count_process_global(const string &label) {
        global_count++;
    }
    /* Count pass: END */

    /* Load pass: START */
    void load_pass() {
        cout << "allocating structures based on block_count: "
            << block_count << endl;
        cout << "allocating structures based on net_count: "
            << net_count << endl;
        block_list = (struct s_block *)malloc(sizeof(struct s_block) * block_count);
        memset(block_list, 0, (sizeof(struct s_block) * block_count));

        net_list = (struct s_net *)malloc(sizeof(struct s_net) * net_count);
        memset(net_list, 0, (sizeof(struct s_net) * net_count));

        subblock_list = (t_subblock **)malloc(sizeof(t_subblock *) * block_count);
        memset(subblock_list, 0, (sizeof(t_subblock *) * block_count));

        subblocks_count = (int *)malloc(sizeof(int) * block_count);
        memset(subblocks_count, 0, (sizeof(int) * block_count));

        p.init();
        p.register_input_process_func(boost::bind(&NetlistReader::load_process_input, this, _1, _2));
        p.register_output_process_func(boost::bind(&NetlistReader::load_process_output, this, _1, _2));
        p.register_funcblock_process_func(boost::bind(&NetlistReader::load_process_funcblock, this, _1, _2, _3, _4));
        p.parse(in_stream);
        load_display_contents();
    }

    void load_process_funcblock(const string &funcblocktype, 
            const string &label, const vector<string> &pins,
            const vector<SubBlock> &subblocks) {
        funcblock_count++;
        for(int i = 0; i < pins.size(); i++) {
            const int &pin_class = pin_class_map[funcblocktype][i];
            if(pins[i] != "open" && pin_class == DRIVER) {
                load_process_net(pins[i]);
                net_index++;
            }
        }
        
        int num_pins = pin_class_map[funcblocktype].size();
        block_list[block_index].nets = (int *)calloc(num_pins, sizeof(int));
        subblocks_count[block_index] = subblocks.size();
        load_process_block(label);
        block_index++;
    }

    void load_process_net(const string &label) {
        net_list[net_index].name = (char *)calloc(label.size() + 1, sizeof(char));
        strcpy(net_list[net_index].name, label.c_str());
        net_map[label] = net_index;
    }

    void load_process_block(const string &label) {
        block_list[block_index].name = (char *)calloc(label.size() + 1, sizeof(char));
        strcpy(block_list[block_index].name, label.c_str());
    }

    void load_process_input(const string &label, const vector<string> &pins) {
        load_process_net(pins[0]);
        net_index++;
        int num_pins = pin_class_map[".input"].size();
        block_list[block_index].nets = (int *)calloc(num_pins, sizeof(int));
        load_process_block(label);
        block_index++;
    }

    void load_process_output(const string &label, const vector<string> &pins) {
        int num_pins = pin_class_map[".output"].size();
        block_list[block_index].nets = (int *)calloc(num_pins, sizeof(int));
        load_process_block(label);
        block_index++;
    }

    void load_display_contents() {
#ifdef VERBOSE
        for(int i = 0; i < block_count; i++) {
            cout << "block[" << i << "]: " << block_list[i].name << " has " << subblocks_count[i] << " subblocks" << endl;
        }
        for(int i = 0; i < net_count; i++) {
            cout << "net[" << i << "]: " << net_list[i].name << endl;
        }
#endif
    }
    /* Load pass: END */

    /* Map pass: START */
    void map_pass() {
        subblock_list = (t_subblock **)calloc(block_count, sizeof(t_subblock*));
        memset(subblock_list, 0, sizeof(t_subblock *) * block_count);

        p.init();
        p.register_input_process_func(boost::bind(&NetlistReader::map_process_input, this, _1, _2));
        p.register_output_process_func(boost::bind(&NetlistReader::map_process_output, this, _1, _2));
        p.register_funcblock_process_func(boost::bind(&NetlistReader::map_process_funcblock, this, _1, _2, _3, _4));
        p.parse(in_stream);
        map_display_contents();
    }

    void map_process_funcblock(const string &funcblocktype, 
            const string &label, const vector<string> &pins,
            const vector<SubBlock> &subblocks) {
        int num_pins = pin_class_map[funcblocktype].size();
        for(int i = 0; i < pins.size(); i++) {
            if(pins[i] != "open") {
                const int &pin_class = pin_class_map[funcblocktype][i];
                if(pin_class == DRIVER) {
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

    void map_process_input(const string &label, const vector<string> &pins) {
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

    void map_process_output(const string &label, const vector<string> &pins) {
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

    void map_display_contents() {
#ifdef VERBOSE
        for(int i = 0; i < block_count; i++) {
            cout << "block[" << i << "]: " << block_list[i].name << " has " << subblocks_count[i] << " subblocks" << endl;
            for(int j = 0; j < 3; j++) {
                cout << "  net[" << j << "]: " << block_list[i].nets[j] << endl;
            }
            for(int j = 0; j < subblocks_count[i]; j++) {
                cout << "  subblock[" << subblock_list[i][j].name << "]" << endl;
            }
        }
        for(int i = 0; i < net_count; i++) {
            cout << "net[" << i << "]: " << net_list[i].name << endl;
        }
#endif
    }
    /* Map pass: END */

    parse_data_t parse() {
        count_pass();

        cout << "------------------------------------------------------------------------" << endl;
        cout << "COUNT PASS:" << endl;
        cout << "  Global count:       " << global_count << endl;
        cout << "  Funcblock count:    " << funcblock_count << endl;
        cout << "  Input count:        " << input_count << endl;
        cout << "  Output count:       " << output_count << endl;
        cout << "  --------------------------" << endl;
        cout << "  Block count:        " << block_count << endl;
        cout << "  Net count:          " << net_count << endl;

        rewind();
        cout << "------------------------------------------------------------------------" << endl;
        cout << "LOAD PASS:" << endl;
        load_pass();
        rewind();
        cout << "------------------------------------------------------------------------" << endl;
        cout << "MAP PASS:" << endl;
        map_pass();
    }
};


#endif
