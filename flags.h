#ifndef FLAGS_H
#define FLAGS_H

// define struct for flags, with each flag being a single bit
struct flags_struct {
    unsigned int c_flag : 1;
    unsigned int d_flag : 1;
    unsigned int h_flag : 1;
    unsigned int i_flag : 1;
    unsigned int l_flag : 1;
    unsigned int p_flag : 1;
};

#endif /* FLAGS_H */
