#ifndef ___BLOCK_HANDLER__HPP___
#define ___BLOCK_HANDLER__HPP___

#include <boost/lexical_cast.hpp>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include "util.h"
#include "vpr_types.h"
#include "NetList.hpp"
#include "VPRNetParser.hpp"

using namespace std;

typedef map<string, int> net_map_t;

struct arch_data_t {
    int const num_types;
    const vector<s_type_descriptor> &block_types;
    t_type_ptr const IO_type;
    int const io_ipin;
    int const io_opin;

    arch_data_t(int num_types,
	     const vector<s_type_descriptor> &block_types,
	     const t_type_ptr IO_type,
	     int io_ipin,
	     int io_opin) :
            num_types(num_types),
            block_types(block_types),
            IO_type(IO_type),
            io_ipin(io_ipin),
            io_opin(io_opin) {}
};

struct parse_data_t {
    vector<Block> &block_list;
    vector<struct s_net> &net_list;
    subblock_grid_t &subblock_list;
    vector<int> subblocks_count;

    parse_data_t(vector<Block> &block_list, 
            vector<struct s_net> &net_list,
            subblock_grid_t &subblock_list)
        : block_list(block_list), net_list(net_list),
            subblock_list(subblock_list) {}
};

typedef map<string, const s_type_descriptor *> pin_class_map_t;

class NetlistReader {
    VPRNetParser p;
    ifstream &in_stream;
    pin_class_map_t pin_class_map;
    net_map_t net_map;

    int funcblock_count;
    int input_count;
    int output_count;
    int global_count;

    int net_count;
    int block_count;

    int net_index;
    int block_index;

    vector<Block> &block_list;
    vector<struct s_net> &net_list;
    subblock_grid_t &subblock_list;
    vector<int> subblocks_count;

    parse_data_t &parse_data;
    arch_data_t const &arch_data;

public:
    NetlistReader(ifstream &in_stream, arch_data_t const &arch_data,
                    parse_data_t &parse_data) : 
            parse_data(parse_data),
            arch_data(arch_data),
            block_list(parse_data.block_list),
            net_list(parse_data.net_list),
            subblock_list(parse_data.subblock_list),
            in_stream(in_stream),
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
        for(int i = 0; i < arch_data.block_types.size(); i++) {
            const s_type_descriptor &t = arch_data.block_types[i];

            pin_class_map[t.name] = &t;
        }
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
            const s_type_descriptor &type = *pin_class_map[funcblocktype];
            const int &pin_class = type.class_inf[type.pin_class[i]].type;
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
        block_list.resize(block_count);
        for(int i = 0; i < block_list.size(); i++) {
            block_list[i].type = NULL;
            block_list[i].x(-1);
            block_list[i].y(-1);
            block_list[i].z(-1);
        }
        net_list.resize(net_count);
        for(int i = 0; i < net_list.size(); i++) {
            net_list[i].num_sinks = 0;
            net_list[i].node_block = NULL;
            net_list[i].node_block_pin = NULL;
            net_list[i].is_global = (boolean)false;
        }

        subblock_list.resize(block_count);

        subblocks_count = vector<int>(block_count, 0);

        p.init();
        p.register_input_process_func(boost::bind(&NetlistReader::load_process_input, this, _1, _2));
        p.register_output_process_func(boost::bind(&NetlistReader::load_process_output, this, _1, _2));
        p.register_funcblock_process_func(boost::bind(&NetlistReader::load_process_funcblock, this, _1, _2, _3, _4));
        p.parse(in_stream);
    }

    void load_process_funcblock(const string &funcblocktype, 
            const string &label, const vector<string> &pins,
            const vector<SubBlock> &subblocks) {
        const s_type_descriptor &type = *pin_class_map[funcblocktype];
        int num_pins = type.num_pins;

        block_list[block_index].type = &type;
        for(int i = 0; i < pins.size(); i++) {
            const int &pin_class = type.class_inf[type.pin_class[i]].type;
            if(pins[i] != "open" && pin_class == DRIVER) {
                load_process_net(pins[i]);
                net_index++;
            }
        }
        
        block_list[block_index].nets = vector<int>(num_pins, OPEN);
        subblocks_count[block_index] = subblocks.size();
        load_process_block(label);
        block_index++;
    }

    void load_process_input(const string &label, const vector<string> &pins) {
        const s_type_descriptor &type = *pin_class_map[".io"];
        int num_pins = type.num_pins;

        load_process_net(pins[0]);
        net_index++;

        block_list[block_index].type = &type;
        block_list[block_index].nets = vector<int>(num_pins, OPEN);
        load_process_block(label);
        block_index++;
    }

    void load_process_output(const string &label, const vector<string> &pins) {
        const s_type_descriptor &type = *pin_class_map[".io"];
        int num_pins = type.num_pins;

        block_list[block_index].type = &type;
        block_list[block_index].nets = vector<int>(num_pins, OPEN);
        load_process_block(label);
        block_index++;
    }

    void load_process_net(const string &label) {
        /*
        net_list[net_index].name = (char *)calloc(label.size() + 1, sizeof(char));
        strcpy(net_list[net_index].name, label.c_str());
        */
        net_list[net_index].name = label;
        net_map[label] = net_index;
    }

    void load_process_block(const string &label) {
        /*
        block_list[block_index].name = (char *)calloc(label.size() + 1, sizeof(char));
        strcpy(block_list[block_index].name, label.c_str());
        */
        block_list[block_index].name = label;
    }

    void load_display_contents() {
        for(int i = 0; i < block_count; i++) {
            cout << "block[" << i << "]: " << block_list[i].name << " has " << subblocks_count[i] << " subblocks" << endl;
        }
        for(int i = 0; i < net_count; i++) {
            cout << "net[" << i << "]: " << net_list[i].name << endl;
        }
    }
    /* Load pass: END */

    /* Map pass: START */
    void map_pass() {
        p.init();
        p.register_input_process_func(boost::bind(&NetlistReader::map_process_input, this, _1, _2));
        p.register_output_process_func(boost::bind(&NetlistReader::map_process_output, this, _1, _2));
        p.register_global_process_func(boost::bind(&NetlistReader::map_process_global, this, _1));
        p.register_funcblock_process_func(boost::bind(&NetlistReader::map_process_funcblock, this, _1, _2, _3, _4));
        p.parse(in_stream);
    }

    void map_process_funcblock(const string &funcblocktype, 
            const string &label, const vector<string> &pins,
            const vector<SubBlock> &subblocks) {
        const s_type_descriptor &type = *pin_class_map[funcblocktype];
        int num_pins = type.num_pins;

        for(int i = 0; i < pins.size(); i++) {
            if(pins[i] != "open") {
                const int &pin_class = type.class_inf[type.pin_class[i]].type;
                if(pin_class == DRIVER) {
                    net_index++;
                }
                block_list[block_index].nets[i] = net_map[pins[i]];
            } else {
                block_list[block_index].nets[i] = OPEN;
            }
        }
        if(subblocks.size() > 0) {
            //subblock_list[block_index] = (t_subblock *)calloc(subblocks_count[block_index], sizeof(t_subblock));
            subblock_list[block_index].resize(subblocks_count[block_index]);
            for(int i = 0; i < subblocks.size(); i++) {
                int max_subblocks = type.max_subblocks;
                int max_subblock_inputs = type.max_subblock_inputs;
                int max_subblock_outputs = type.max_subblock_outputs;

                t_subblock &sb = subblock_list[block_index][i];
                sb.inputs = (int*)calloc(max_subblock_inputs, sizeof(int));
                sb.outputs = (int*)calloc(max_subblock_outputs, sizeof(int));

                sb.name = label;
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
        for(int i = 0; i < pin_class_map[".io"]->num_pins; i++) {
            block_list[block_index].nets[i] = OPEN;
        }
        block_list[block_index].nets[arch_data.io_opin] = net_map[pins[0]];

        net_index++;
        block_index++;
    }

    void map_process_output(const string &label, const vector<string> &pins) {
        for(int i = 0; i < pin_class_map[".io"]->num_pins; i++) {
            block_list[block_index].nets[i] = OPEN;
        }
        block_list[block_index].nets[arch_data.io_ipin] = net_map[pins[0]];

        block_index++;
    }


    void map_process_global(const string &label) {
        for(int i = 0; i < net_count; i++) {
            if(label == net_list[i].name) {
                net_list[i].is_global = (boolean)1;
                break;
            }
        }
    }

    void map_display_contents() {
        for(int i = 0; i < block_count; i++) {
            const s_type_descriptor &type = *block_list[i].type;
            cout << "block[" << i << "]: " << block_list[i].name << " has " << subblocks_count[i] << " subblocks" << endl;
            for(int j = 0; j < type.num_pins; j++) {
                cout << "  net[" << j << "]: " << block_list[i].nets[j] << endl;
            }
            for(int j = 0; j < subblocks_count[i]; j++) {
                cout << "  subblock[" << subblock_list[i][j].name << "]" << endl;
            }
        }
        cout << "NETS:" << endl;
        for(int i = 0; i < net_count; i++) {
            cout << "  net[" << i << "]: " << net_list[i].name << endl;
        }
    }
    /* Map pass: END */

    //vpr::NetList parse() {
    void parse() {
        count_pass();
        rewind();
        load_pass();
        rewind();
        map_pass();

        /* Builds mappings from each netlist to the blocks contained */
        sync_nets_to_blocks();

        /**********************************************************************************/
        /* Send values back to caller */
        t_subblock_data subblock_data;
        subblock_data.subblock_inf = subblock_list;
        subblock_data.num_subblocks_per_block = subblocks_count;
        /**********************************************************************************/

        vpr::NetList n(net_list, block_list, subblock_data);

        /**********************************************************************************/
        /* Send values back to caller */
        /**********************************************************************************/

        parse_data.subblocks_count = subblocks_count;
        //return n;
    }

    /* This function updates the nets list to point back to blocks list */
    void sync_nets_to_blocks() {
        using namespace std;

        int i, j, k, l;
        t_type_ptr cur_type;

        /* Count the number of sinks for each net */
        for(j = 0; j < block_list.size(); ++j) {
            cur_type = block_list[j].type;
            for(k = 0; k < cur_type->num_pins; ++k) {
                if(RECEIVER == cur_type->class_inf[cur_type->pin_class[k]].type) {
                    int i = block_list[j].nets[k];
                    if(OPEN != i) {
                        ++net_list[i].num_sinks;
                    }
                }
            }
        }

        /* Alloc and load block lists of nets */
        vector<int> net_l(net_list.size(), 1); /* First sink goes at position 1, since 0 is for driver */

        for(j = 0; j < block_list.size(); ++j) {
            cur_type = block_list[j].type;
            for(k = 0; k < cur_type->num_pins; ++k) {
                int i = block_list[j].nets[k];
                if(OPEN != i) {
                    /* The list should be num_sinks + 1 driver. Re-alloc if already allocated. */
                    int num_sinks = net_list[i].num_sinks;
                    if(NULL == net_list[i].node_block) {
                        net_list[i].node_block = (int *)my_malloc(sizeof(int) * (num_sinks + 1));
                    }
                    if(NULL == net_list[i].node_block_pin) {
                        net_list[i].node_block_pin = (int *)my_malloc(sizeof(int) * (num_sinks + 1));
                    }
                    if(RECEIVER == cur_type->class_inf[cur_type->pin_class[k]].type) {
                        l = net_l[i];
                        net_list[i].node_block[l] = j;
                        net_list[i].node_block_pin[l] = k;
                        ++net_l[i];
                    } else {
                        assert(DRIVER == cur_type->class_inf[cur_type->pin_class[k]].type);
                        net_list[i].node_block[0] = j;
                        net_list[i].node_block_pin[0] = k;
                    }
                }
            }
        }
    }


};


#endif
