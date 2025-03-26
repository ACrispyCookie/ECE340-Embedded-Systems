
module LZCounter (
    input [23:0] bits,
    output reg [4:0] leadingZeroCount
);
    always @(bits) begin
        casex (bits)
            24'b1xxxxxxxxxxxxxxxxxxxxxxx: leadingZeroCount = 5'd0;
            24'b01xxxxxxxxxxxxxxxxxxxxxx: leadingZeroCount = 5'd1;
            24'b001xxxxxxxxxxxxxxxxxxxxx: leadingZeroCount = 5'd2;
            24'b0001xxxxxxxxxxxxxxxxxxxx: leadingZeroCount = 5'd3;
            24'b00001xxxxxxxxxxxxxxxxxxx: leadingZeroCount = 5'd4;
            24'b000001xxxxxxxxxxxxxxxxxx: leadingZeroCount = 5'd5;
            24'b0000001xxxxxxxxxxxxxxxxx: leadingZeroCount = 5'd6;
            24'b00000001xxxxxxxxxxxxxxxx: leadingZeroCount = 5'd7;
            24'b000000001xxxxxxxxxxxxxxx: leadingZeroCount = 5'd8;
            24'b0000000001xxxxxxxxxxxxxx: leadingZeroCount = 5'd9;
            24'b00000000001xxxxxxxxxxxxx: leadingZeroCount = 5'd10;
            24'b000000000001xxxxxxxxxxxx: leadingZeroCount = 5'd11;
            24'b0000000000001xxxxxxxxxxx: leadingZeroCount = 5'd12;
            24'b00000000000001xxxxxxxxxx: leadingZeroCount = 5'd13;
            24'b000000000000001xxxxxxxxx: leadingZeroCount = 5'd14;
            24'b0000000000000001xxxxxxxx: leadingZeroCount = 5'd15;
            24'b00000000000000001xxxxxxx: leadingZeroCount = 5'd16;
            24'b000000000000000001xxxxxx: leadingZeroCount = 5'd17;
            24'b0000000000000000001xxxxx: leadingZeroCount = 5'd18;
            24'b00000000000000000001xxxx: leadingZeroCount = 5'd19;
            24'b000000000000000000001xxx: leadingZeroCount = 5'd20;
            24'b0000000000000000000001xx: leadingZeroCount = 5'd21;
            24'b00000000000000000000001x: leadingZeroCount = 5'd22;
            24'b000000000000000000000001: leadingZeroCount = 5'd23;
            default: leadingZeroCount = 5'd24;
        endcase
    end

endmodule
