`timescale 1ns/1ps

module LZCounter_tb;
    reg [23:0] bits;
    wire [4:0] lz_count;

    // Instantiate the LZCounter module
    LZCounter uut (
        .bits(bits),
        .LeadingZeroCount(lz_count)
    );

    initial begin
        // Apply test vectors
        bits = 24'b000000000000000000000000; #10; // Expect 24 leading zeros
        bits = 24'b000000000000000000000001; #10; // Expect 23 leading zeros
        bits = 24'b000000000000000000000011; #10; // Expect 22 leading zeros
        bits = 24'b000000000000000000000111; #10; // Expect 21 leading zeros

        bits = 24'b000000000000000000001000; #10; // Expect 20 leading zeros
        bits = 24'b000000000000000000010000; #10; // Expect 19 leading zeros
        bits = 24'b000000000000000000100000; #10; // Expect 18 leading zeros
        bits = 24'b000000000000000001000000; #10; // Expect 17 leading zeros

        bits = 24'b000000000000000010001100; #10; // Expect 16 leading zeros
        bits = 24'b000000000000000100010010; #10; // Expect 15 leading zeros
        bits = 24'b000000000000001011100000; #10; // Expect 14 leading zeros
        bits = 24'b000000000000011111111111; #10; // Expect 13 leading zeros

        bits = 24'b000000000000100000000000; #10; // Expect 12 leading zeros
        bits = 24'b000000000001000000111111; #10; // Expect 11 leading zeros
        bits = 24'b000000000010000000000000; #10; // Expect 10 leading zeros
        bits = 24'b000000000100000000000000; #10; // Expect 9 leading zeros

        bits = 24'b000000001000000000000000; #10; // Expect 8 leading zeros
        bits = 24'b000000010000000000000000; #10; // Expect 7 leading zeros
        bits = 24'b000000100000000000000000; #10; // Expect 6 leading zeros
        bits = 24'b000001000000000000000000; #10; // Expect 5 leading zeros

        bits = 24'b000010000000000000000000; #10; // Expect 4 leading zeros
        bits = 24'b000100000000000000000000; #10; // Expect 3 leading zeros
        bits = 24'b001000000000000000000000; #10; // Expect 2 leading zeros
        bits = 24'b010000000000000000000000; #10; // Expect 1 leading zero

        bits = 24'b100000000000000000000000; #10; // Expect 0 leading zeros


        // Finish simulation
        #10;
        $stop;
    end
endmodule
