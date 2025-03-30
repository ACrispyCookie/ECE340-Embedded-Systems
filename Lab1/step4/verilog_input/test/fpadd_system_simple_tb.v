`timescale 1ns / 1ps

module fpadd_system_tb;
    // Testbench signals
    reg clk;
    reg reset;
    reg button;
    wire [7:0] leds;
    wire an0, a0, b0, c0, d0, e0, f0, g0;
    wire an1, a1, b1, c1, d1, e1, f1, g1;

    // Instantiate the DUT (Device Under Test)
    fpadd_system_simple dut (
        .clk(clk),
        .reset(reset),
        .button(button),
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
        button = 0;

        // Apply reset
        #100_000_200 reset = 0;

        // Press button to trigger input selection
        #50 button = 1;
        #10 button = 0;

        // Wait and observe output
        #100;

        // Finish simulation
        $stop;
    end
endmodule