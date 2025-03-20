module LZCounter (
    input [23:0] bits,
    output [4:0] lz_count
);
    wire [5:0] zrs;
    wire [1:0] count [5:0];
    wire [2:0] all_zero_count;

    LZCx4 LZC_inst1 (.bits(bits[23:20]), .count(count[0]), .all_zero(zrs[0]));
    LZCx4 LZC_inst2 (.bits(bits[19:16]), .count(count[1]), .all_zero(zrs[1]));
    LZCx4 LZC_inst3 (.bits(bits[15:12]), .count(count[2]), .all_zero(zrs[2]));
    LZCx4 LZC_inst4 (.bits(bits[11:8]),  .count(count[3]), .all_zero(zrs[3]));
    LZCx4 LZC_inst5 (.bits(bits[7:4]),   .count(count[4]), .all_zero(zrs[4]));
    LZCx4 LZC_inst6 (.bits(bits[3:0]),   .count(count[5]), .all_zero(zrs[5]));
    LZEx6 LZEx6_inst1 (.zrs(zrs), .count(all_zero_count));
    
    assign lz_count = &zrs == 1'b1 ? 5'd24 : {all_zero_count, count[all_zero_count]};
endmodule