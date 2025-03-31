/*
    The Character Decoder module converts a 4-bit binary input into a 7-segment display output.
    The output is represented by the signals a, b, c, d, e, f, and g, which correspond to the 
    segments of the display, where '1' means the segment is on and '0' means it is off.

    ?Input:
        data_in: 4-bit binary input representing a hexadecimal digit (0-15).
    ?Output:
        a, b, c, d, e, f, g: 7-bit output representing the segments of the display.
    
    *The mapping is as follows:
        - 0: 7'b1111110 (abcdef)
        - 1: 7'b0110000 (bc)
        - 2: 7'b1101101 (abde)
        - 3: 7'b1111001 (abd)
        - 4: 7'b0110011 (bcfg)
        - 5: 7'b1011011 (acdfg)
        - 6: 7'b1011111 (acdefg)
        - 7: 7'b1110000 (abc)
        - 8: 7'b1111111 (abcdefg)
        - 9: 7'b1111011 (abcfg)
        - A: 7'b1110111 (abcefg)
        - B: 7'b0011111 (cdefg)
        - C: 7'b1001110 (adef)
        - D: 7'b0111101 (bcdefg)
        - E: 7'b1001111 (adefg)
        - F: 7'b1000111 (aefg)
*/

module char_decoder (
    input [3:0] data_in,
    output reg a, b, c, d, e, f, g
);

always @(data_in) begin
    case (data_in)
        4'b0000: {a, b, c, d, e, f, g} = 7'b1111110;
        4'b0001: {a, b, c, d, e, f, g} = 7'b0110000;
        4'b0010: {a, b, c, d, e, f, g} = 7'b1101101;
        4'b0011: {a, b, c, d, e, f, g} = 7'b1111001;
        4'b0100: {a, b, c, d, e, f, g} = 7'b0110011;
        4'b0101: {a, b, c, d, e, f, g} = 7'b1011011;
        4'b0110: {a, b, c, d, e, f, g} = 7'b1011111;
        4'b0111: {a, b, c, d, e, f, g} = 7'b1110000;
        4'b1000: {a, b, c, d, e, f, g} = 7'b1111111;
        4'b1001: {a, b, c, d, e, f, g} = 7'b1111011;
        4'b1010: {a, b, c, d, e, f, g} = 7'b1110111;
        4'b1011: {a, b, c, d, e, f, g} = 7'b0011111;
        4'b1100: {a, b, c, d, e, f, g} = 7'b1001110;
        4'b1101: {a, b, c, d, e, f, g} = 7'b0111101;
        4'b1110: {a, b, c, d, e, f, g} = 7'b1001111;
        4'b1111: {a, b, c, d, e, f, g} = 7'b1000111;
        default: {a, b, c, d, e, f, g} = 7'b1111111;
    endcase
end
    
endmodule