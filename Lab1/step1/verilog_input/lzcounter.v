module LZCounter (
    input [23:0] bits,
    output reg [4:0] leadingZeroCount
);

    always @(bits) begin
        if (bits[23] == 1) leadingZeroCount = 5'd0;
        else if (bits[22] == 1) leadingZeroCount = 5'd1;
        else if (bits[21] == 1) leadingZeroCount = 5'd2;
        else if (bits[20] == 1) leadingZeroCount = 5'd3;
        else if (bits[19] == 1) leadingZeroCount = 5'd4;
        else if (bits[18] == 1) leadingZeroCount = 5'd5;
        else if (bits[17] == 1) leadingZeroCount = 5'd6;
        else if (bits[16] == 1) leadingZeroCount = 5'd7;
        else if (bits[15] == 1) leadingZeroCount = 5'd8;
        else if (bits[14] == 1) leadingZeroCount = 5'd9;
        else if (bits[13] == 1) leadingZeroCount = 5'd10;
        else if (bits[12] == 1) leadingZeroCount = 5'd11;
        else if (bits[11] == 1) leadingZeroCount = 5'd12;
        else if (bits[10] == 1) leadingZeroCount = 5'd13;
        else if (bits[9] == 1) leadingZeroCount = 5'd14;
        else if (bits[8] == 1) leadingZeroCount = 5'd15;
        else if (bits[7] == 1) leadingZeroCount = 5'd16;
        else if (bits[6] == 1) leadingZeroCount = 5'd17;
        else if (bits[5] == 1) leadingZeroCount = 5'd18;
        else if (bits[4] == 1) leadingZeroCount = 5'd19;
        else if (bits[3] == 1) leadingZeroCount = 5'd20;
        else if (bits[2] == 1) leadingZeroCount = 5'd21;
        else if (bits[1] == 1) leadingZeroCount = 5'd22;
        else if (bits[0] == 1) leadingZeroCount = 5'd23;
        else leadingZeroCount = 5'd24; // If all bits are zero
    end
endmodule
