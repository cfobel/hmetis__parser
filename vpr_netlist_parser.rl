#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <string.h>
#include <stdlib.h>

using namespace std;

#define DEF_BUFSIZE 8192

%%{
	machine VPRNetParser;

	action start_line {
		ls = fpc;
		ts = fpc;
	}

	action end_input {
        process_input();
	}

	action end_output {
        process_output();
	}

	action end_clb {
        if(ts != be) {
            ls = buf;
        }
        ts = 0;
        process_clb();
	}

	action end_block {
        be = fpc;
        te = fpc;
	}
    
    action start_pinlist {
        pin_list.clear();
        in_pin_list = true;
    }
    
    action end_pinlist {
        in_pin_list = false;
    }
    
    action start_pin {
        pin_start = fpc;
    }
    
    action end_pin {
        length = fpc - pin_start;
        if(length < 0) {
            pin_start = buf + (pin_start - be);
        }
        if(in_pin_list) pin_list.push_back(string(pin_start, fpc - pin_start));
    }

    action start_label {
        label_start = fpc;
    }
    
    action end_label {
        length = fpc - label_start;
        if(length < 0) {
            label_start = buf + (label_start - be);
        }
        label = string(label_start, fpc - label_start);
    }

	# Words in a line.
	word = ^[ \t\n]+;

	# The whitespace separating words in a line.
	whitespace = [ \t];

	label_char = alnum | [\[\]_:\\];
	label = label_char (label_char)* $1 %0;
    block_label = label >start_label %end_label;
	paddedlabel = whitespace+ block_label whitespace*;
	properlabel = (label - "open") >start_pin %end_pin;
	pinlabel = ("open" | properlabel);

	pins = whitespace+ pinlabel ('\\\n' | whitespace+ | pinlabel)* $1 %0;
	pinlist = ( 'pinlist:' pins ) >start_pinlist %end_pinlist;
	subblock = ( 'subblock:' pins );
    comment = ( '#' (whitespace* word)** );
    endofline = ( comment? whitespace* '\n' );
	emptyline = whitespace* endofline;

	global =     ( '.global' paddedlabel endofline );
	input =      ( '.input'  paddedlabel endofline 
                        whitespace* pinlist endofline ) 
                >start_line %end_input;
	output =     ( '.output' paddedlabel endofline 
                        whitespace* pinlist endofline ) 
                >start_line %end_output;
	logicblock = ( '.clb'    paddedlabel endofline 
                        whitespace* pinlist endofline 
                        (whitespace* subblock endofline)+ ) 
                >start_line %end_clb;

	# Any number of lines.
	main := (emptyline %end_block | global | input | output | logicblock)+;
}%%

typedef boost::function<void (const string &x, const vector<string> &y)> process_func_t;

class VPRNetParser {
    /* Regal data ****************************************/
    %% write data nofinal;
    /* Regal data: end ***********************************/

    vector<char> buf_vector;
    char* buf;
    int BUFSIZE;

	const char *ls;
	const char *ts;
	const char *te;
	const char *be;
	const char *pin_start;
	const char *label_start;

	int cs;
	int have;
	int length;

    vector<string> pin_list;
    string label;
    bool in_pin_list;
    process_func_t clb_process_func;
    process_func_t input_process_func;
    process_func_t output_process_func;

public:
    VPRNetParser() {
        buf_vector = vector<char>(DEF_BUFSIZE);
    }
    VPRNetParser(int buffer_size) {
        buf_vector = vector<char>(buffer_size);
    }
	void init();
	void parse();
	void display_pins();

    void register_input_process_func(process_func_t fun) { input_process_func = fun; }
    void process_input() { input_process_func(label, pin_list); }
    void register_output_process_func(process_func_t fun) { output_process_func = fun; }
    void process_output() { output_process_func(label, pin_list); }
    void register_clb_process_func(process_func_t fun) { clb_process_func = fun; }
    void process_clb() { clb_process_func(label, pin_list); }
};

void VPRNetParser::display_pins() {
    cout << "Pins:";
    for(int i = 0; i < pin_list.size(); i++) {
        cout << " " << pin_list[i];
    }
    cout << endl;
}


void VPRNetParser::init() {
    buf = &buf_vector[0];
    BUFSIZE = buf_vector.size();

	%% write init;
}

void VPRNetParser::parse() {
    bool done = false;
    int i = 0;
    have = 0;
    while ( !done ) {
        /* How much space is in the buffer? */
        int space = BUFSIZE - have;
        if ( space == 0 ) {
            /* Buffer is full. */
            cerr << "TOKEN TOO BIG" << endl;
            exit(1);
        }
        /* Read in a block after any data we already have. */
        char *p = buf + have;
        cin.read( p, space );
        int len = cin.gcount();
        char *pe = p + len;
        char *eof = 0;

        /* If no data was read indicate EOF. */
        if ( len == 0 ) {
            eof = pe;
            done = true;
        } else {
            %% write exec;

            if ( cs == VPRNetParser_error ) {
                /* Machine failed before finding a token. */
                cerr << "PARSE ERROR" << endl;
                exit(1);
            }
            if ( ts == 0 ) {
                have = 0;
            } else {
                /* There is a prefix to preserve, shift it over. */
                have = pe - ts;
                memmove( buf, ts, have );
                te = buf + (te-ts);
                ts = buf;
            }
        }
    }
}

class BlockHandler {
public:
    void process_clb(const string &label, const vector<string> &pins) {
        cout << "CLB: " << label << endl;
        cout << "  Pins: ";
        for(int i = 0; i < pins.size(); i++) {
            cout << " " << pins[i];
        }
        cout << endl;
    }

    void process_input(const string &label, const vector<string> &pins) {
        cout << "input: " << label << endl;
        cout << "  Pins: ";
        for(int i = 0; i < pins.size(); i++) {
            cout << " " << pins[i];
        }
        cout << endl;
    }

    void process_output(const string &label, const vector<string> &pins) {
        cout << "output: " << label << endl;
        cout << "  Pins: ";
        for(int i = 0; i < pins.size(); i++) {
            cout << " " << pins[i];
        }
        cout << endl;
    }
};

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
