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
	output reg[31:0] o_fpAdd_out
);

	// Internal Signals
	reg [31:0] regA, regB;
	reg [24:0] mantSum;
	wire signed [7:0] expDiff;
	wire [4:0] leadingZeroCount;
	reg [23:0] shiftedMantA, shiftedMantB;

	// Final / Calculated Signals
	reg finalSign;
	reg [7:0] finalExp;
	reg [22:0] finalMantissa;

    // Pipelined Signals
	reg pl_finalSign;
	reg [24:0] pl_mantSum;
	reg [31:0] pl_regA, pl_regB;
	
	assign expDiff = regA[30:23] - regB[30:23];

	LZCounter LCZ_inst(.bits(pl_mantSum[23:0]), .leadingZeroCount(leadingZeroCount));

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            pl_mantSum <= 25'b0;
            pl_finalSign <= 1'b0;
            pl_regA <= 32'b0;
            pl_regB <= 32'b0;
        end else begin
            pl_mantSum <= mantSum;
            pl_finalSign <= finalSign;
            pl_regA <= regA;
            pl_regB <= regB;
        end
    end

	// Register the two inputs, and use regA and regB in the combinational logic. 
	always @(posedge clk or posedge reset) begin
		if (reset == 1'b1) begin
			regA <= 32'b0;
			regB <= 32'b0;
			o_fpAdd_out <= 32'b0;
		end else begin
			regA <= i_fpAdd_regA;
			regB <= i_fpAdd_regB;
			o_fpAdd_out <= {pl_finalSign, finalExp, finalMantissa};
		end
	end

	// Shift Mantissa bits until same exponents for addition
	always @(regA or regB or expDiff) begin
		if (regA[30:23] > regB[30:23]) begin
			shiftedMantA = regA[30:0] != 31'b0 ? {1'b1, regA[22:0]} : {1'b0, regA[22:0]};
			shiftedMantB = (regB[30:0] != 31'b0 ? {1'b1, regB[22:0]} : {1'b0, regB[22:0]}) >> expDiff;
		end else if (regA[30:23] < regB[30:23]) begin
			shiftedMantA = (regA[30:0] != 31'b0 ? {1'b1, regA[22:0]} : {1'b0, regA[22:0]}) >> -expDiff;
			shiftedMantB = regB[30:0] != 31'b0 ? {1'b1, regB[22:0]} : {1'b0, regB[22:0]};
		end else begin
			shiftedMantA = regA[30:0] != 31'b0 ? {1'b1, regA[22:0]} : {1'b0, regA[22:0]};
			shiftedMantB = regB[30:0] != 31'b0 ? {1'b1, regB[22:0]} : {1'b0, regB[22:0]};
		end
	end

	// Add / Subtract Mantissa bits 
	always @(regA or regB or shiftedMantA or shiftedMantB) begin
		if (regA[31] ^ regB[31]) begin
			if (shiftedMantA > shiftedMantB) begin
				mantSum = shiftedMantA - shiftedMantB;
			end else begin
				mantSum = shiftedMantB - shiftedMantA;
			end
		end else begin
			mantSum = shiftedMantA + shiftedMantB;
		end
	end

	// Final Calculations
	always @(pl_regA or pl_regB or pl_mantSum or leadingZeroCount) begin
		if (pl_mantSum[24] == 1'b1) begin
			finalMantissa = pl_mantSum >> 1;
			finalExp = pl_regA[30:23] > pl_regB[30:23] ? pl_regA[30:23] + 1 : pl_regB[30:23] + 1;
		end else if (pl_mantSum != 25'b0) begin
			finalMantissa = pl_mantSum << leadingZeroCount;
			finalExp = pl_regA[30:23] > pl_regB[30:23] ? pl_regA[30:23] - leadingZeroCount : pl_regB[30:23] - leadingZeroCount;
		end else begin
			finalMantissa = 23'b0;
			finalExp = 8'b0;
		end
	end

	// Final Sign Calculations
	always @(regA or regB or shiftedMantA or shiftedMantB) begin
		if (regA[31] == regB[31]) begin
			finalSign = regA[31];
		end else if (shiftedMantA > shiftedMantB) begin
			finalSign = regA[31];
		end else begin
			finalSign = regB[31];
		end
	end
endmodule

