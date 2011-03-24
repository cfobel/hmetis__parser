#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

#define BUFSIZE 8192

class VPRNetParser {
    char buf[BUFSIZE];

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

public:
	void init();
	void parse();
	void display_pins();
};

void VPRNetParser::display_pins() {
    cout << "Pins:";
    for(int i = 0; i < pin_list.size(); i++) {
        cout << " " << pin_list[i];
    }
    cout << endl;
}


%%{
	machine VPRNetParser;

	action start_line {
		ls = fpc;
		ts = fpc;
	}

	action end_input {
		cout << "input: " << label << endl;
        display_pins();
	}

	action end_output {
		cout << "output: " << label << endl;
        display_pins();
	}

	action end_clb {
        if(ts != be) {
            ls = buf;
        }
		cout << "clb: " << label << endl;
        display_pins();
        ts = 0;
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

%% write data nofinal;

void VPRNetParser::init() {
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
                cout << "Have none left..." << endl;
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

int main() {
    VPRNetParser parser;
    cout << "Initialize" << endl;

	parser.init();
    parser.parse();
    cout << "Done" << endl;
	return 0;
}
