module LZCounter (
    input [23:0] bits,
    output reg [4:0] LeadingZeroCount
);
    wire [5:0] zrs;
    wire [1:0] count[5:0];  // Fixed indexing to match 6 elements
    wire [2:0] all_zero_count;
    wire [1:0] selected_count;

    // Instantiate 4-bit Leading Zero Counters
    LZCx4 LZC_inst1 (.bits(bits[23:20]), .count(count[5]), .all_zero(zrs[0]));
    LZCx4 LZC_inst2 (.bits(bits[19:16]), .count(count[4]), .all_zero(zrs[1]));
    LZCx4 LZC_inst3 (.bits(bits[15:12]), .count(count[3]), .all_zero(zrs[2]));
    LZCx4 LZC_inst4 (.bits(bits[11:8]),  .count(count[2]), .all_zero(zrs[3]));
    LZCx4 LZC_inst5 (.bits(bits[7:4]),   .count(count[1]), .all_zero(zrs[4]));
    LZCx4 LZC_inst6 (.bits(bits[3:0]),   .count(count[0]), .all_zero(zrs[5]));

    // Count how many 4-bit blocks are all zeros
    LZEx6 LZEx6_inst1 (.zrs(zrs), .count(all_zero_count));

    // Mux to select the correct count value
    always @(*) begin
        case (all_zero_count)
            3'd0: LeadingZeroCount = {all_zero_count, count[5]};
            3'd1: LeadingZeroCount = {all_zero_count, count[4]};
            3'd2: LeadingZeroCount = {all_zero_count, count[3]};
            3'd3: LeadingZeroCount = {all_zero_count, count[2]};
            3'd4: LeadingZeroCount = {all_zero_count, count[1]};
            3'd5: LeadingZeroCount = {all_zero_count, count[0]};
            default: LeadingZeroCount = 24'd24;
        endcase
    end
endmodule
