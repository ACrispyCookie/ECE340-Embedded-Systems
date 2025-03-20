`timescale 1ns / 1ps
module tb_fpadd;
reg clk;
reg rst;
reg [31:0] A;
reg [31:0] B;
wire [31:0] out;

localparam CLK_PERIOD = 10;
always #(CLK_PERIOD/2) clk=~clk;

fpadd_single DUT(.clk(clk), .reset(rst), .reg_A(A), .reg_B(B), .out(out));

initial begin
    clk=0;
    rst=0;
    #300 rst=1;
    #100 rst=0;
    #100 A=32'h3f800000; B=32'h3f800000; // 1.0 + 1.0 = 2.0
    #100 A=32'h3f800000; B=32'hbf800000; // 1.0 + -1.0 = 0.0
    #100 A=32'h3f800000; B=32'h40000000; // 1.0 + 2.0 = 3.0
    #100 A=32'h3f800000; B=32'h40400000; // 1.0 + 3.0 = 4.0
    #100 $finish;
end

endmodule