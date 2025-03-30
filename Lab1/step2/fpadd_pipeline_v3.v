`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: UTH
// 
// Design Name: 
// Module Name:   fpadd_single 
// Project Name: 32 bit Floating Point Unit - Add
// Target Devices: Zedboard
// Tool versions: Vivado 2020.2
//
// Description: 32-bit FP adder with regA single pipeline stage (everything happens in one cycle)
//  The module does not check the input for subnormal and NaN numbers, 
//  and assumes that the two inputs are normal FP32 numbers with 0<exp<255.
//  We also assume that the output does not overflow or undeflow, so there is no need to check for these conditions.
//  An FP32 number has 1 sign bit, 8 exponent bits(biased by 127), and 23 mantissa bits.
//////////////////////////////////////////////////////////////////////////////////
module fpadd_pipeline (
	input clk,
    input reset,
    input [31:0] i_fpAdd_regA, 
    input [31:0] i_fpAdd_regB,  
	output [31:0] o_fpAdd_out
);
    // Inputs
    wire signA, signB;
    wire [7:0] expA, expB;
    wire [22:0] mantA, mantB;

    reg [7:0] finalExp;
    reg [22:0] finalMantissa;
    reg [24:0] regsMantSum;
    wire [7:0] expDiff;
    wire [4:0] leadingZeroCount;

    reg largerSign, smallerSign;
    reg [7:0] largerExp, smallerExp;
    reg [23:0] largerMant, smallerMant;
    wire [23:0] smallShiftMant;
    wire [63:0] pipe_out;
    wire [32:0] pipe_out_3;

    wire [24:0] regsMantSumPl;
    wire [7:0] largerExpPl;

    assign expDiff = largerExp - smallerExp;
    assign smallShiftMant = smallerMant >> expDiff;

	LZCounter LCZ_inst(.bits(regsMantSumPl[23:0]), .leadingZeroCount(leadingZeroCount));
    
    // Pipe Stage 1
    pipeline #(.WIDTH(64)) 
        pipeline_inst (.clk(clk), .reset(reset), .in({i_fpAdd_regA, i_fpAdd_regB}), .out(pipe_out));

    assign {signA, expA, mantA, signB, expB, mantB} = pipe_out;

    always @(signA or signB or expA or expB or mantA or mantB) begin
        if ({expA, mantA} > {expB, mantB}) begin
            largerSign = signA;
            largerExp = expA;
            largerMant = {|expA, mantA};
            smallerSign = signB;
            smallerExp = expB;
            smallerMant = {|expB, mantB};
        end else begin
            largerSign = signB;
            largerExp = expB;
            largerMant = {|expB, mantB};
            smallerSign = signA;
            smallerExp = expA;
            smallerMant = {|expA, mantA};
        end
    end

    // Pipe Stage 2
    pipeline #(.WIDTH(33)) 
        pipeline_inst3 (.clk(clk), .reset(reset), .in({largerExp, regsMantSum}), .out(pipe_out_3));

    assign {largerExpPl, regsMantSumPl} = pipe_out_3;

    // Perform addition/subtraction based on sign
    always @(*) begin 
        if (largerSign ^ smallerSign) begin
            regsMantSum = largerMant - smallShiftMant;
        end else begin
            regsMantSum = largerMant + smallShiftMant;
        end
    end

    // Normalize result
    always @(*) begin
        if (regsMantSumPl[24]) begin // Overflow
            finalMantissa = regsMantSumPl >> 1;
            finalExp = largerExpPl + 1;
        end else if (|regsMantSumPl) begin
            finalMantissa = regsMantSumPl << leadingZeroCount;
            finalExp = largerExpPl - leadingZeroCount;
        end else begin
            finalMantissa = 23'b0;
			finalExp = 8'b0;
        end
    end
    // Pipe Stage 3
    pipeline #(.WIDTH(32)) 
        pipeline_inst2 (.clk(clk), .reset(reset), .in({largerSign, finalExp, finalMantissa}), .out(o_fpAdd_out));
endmodule

