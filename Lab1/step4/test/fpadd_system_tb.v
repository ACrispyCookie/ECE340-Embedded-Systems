`timescale 1ns / 1ps

module fpadd_system_tb;
    // Testbench signals
    reg clk;
    reg reset;
    reg [1:0] mem_select;
    reg [1:0] display_select;
    wire [7:0] leds;
    wire an0, a0, b0, c0, d0, e0, f0, g0;
    wire an1, a1, b1, c1, d1, e1, f1, g1;

    // Instantiate the DUT (Device Under Test)
    fpadd_system dut (
        .clk(clk),
        .reset(reset),
        .mem_select(mem_select),
        .display_select(display_select),
        .leds(leds),
        .an0(an0), .a0(a0), .b0(b0), .c0(c0), .d0(d0), .e0(e0), .f0(f0), .g0(g0),
        .an1(an1), .a1(a1), .b1(b1), .c1(c1), .d1(d1), .e1(e1), .f1(f1), .g1(g1)
    );

    // Clock generation
    always #5 clk = ~clk;  // 10ns period -> 100 MHz clock

    // Test sequence
    initial begin
        // Initialize signals
        clk = 0;
        reset = 1;
        mem_select = 2'b00;
        display_select = 2'b00;

        // Apply reset
        #100_000_100 reset = 0;
        
        #1000 mem_select = 2'b10;
        #110 mem_select = 2'b00;

        // Wait and observe output
        #100;

        // Finish simulation
        $stop;
    end
endmodule