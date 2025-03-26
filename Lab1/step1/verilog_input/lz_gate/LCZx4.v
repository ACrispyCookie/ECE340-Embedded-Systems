module LZCx4 (
    input [3:0] bits,
    output all_zero,
    output [1:0] count
);
    
    assign count[0] = (~bits[3] & bits[2]) | (~bits[3] & ~bits[1]); 
    assign count[1] = ~(bits[3] | bits[2]);
    assign all_zero = ~(bits[3] | bits[2] | bits[1] | bits[0]);

endmodule