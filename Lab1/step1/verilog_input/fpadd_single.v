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
// Description: 32-bit FP adder with a single pipeline stage (everything happens in one cycle)
//  The module does not check the input for subnormal and NaN numbers, 
//  and assumes that the two inputs are normal FP32 numbers with 0<exp<255.
//  We also assume that the output does not overflow or undeflow, so there is no need to check for these conditions.
//  An FP32 number has 1 sign bit, 8 exponent bits(biased by 127), and 23 mantissa bits.
//////////////////////////////////////////////////////////////////////////////////
module fpadd_single (input clk,
                     input reset,
                     input [31:0] reg_A, 
                     input [31:0] reg_B,  
		     		 output reg[31:0] out);

	reg [31:0] A, B;
	reg [22:0] shifted_mantissa_A, shifted_mantissa_B;
	reg new_sign;
	wire signed [7:0] exp_diff;
	wire [22:0] mantissa_sum;
	wire [31:0] result;

	assign exp_diff = A[30:23] - B[30:23];
	assign mantissa_sum = shifted_mantissa_A + shifted_mantissa_B;
	assign result = {new_sign, A[30:23], mantissa_sum};
				     	
	// Register the two inputs, and use A and B in the combinational logic. 
	always @(posedge clk or posedge reset) begin
		if (reset == 1'b1) begin
			A <= 32'b0;
			B <= 32'b0;
			out <= 32'b0;
		end else begin
			A <= reg_A;
			B <= reg_B;
			out <= result;
		end
	end
		
	//Combinational Logic to (a) compare and adjust the exponents, 
	//                       (b) shift appropriately the mantissa if necessary, 
	//                       (c) add the two mantissas, and
	//                       (d) perform post-normalization. 
	//                           Make sure to check explicitly for zero output. 
	always @(A or B or exp_diff) begin
		if (A[30:23] > B[30:23]) begin
			shifted_mantissa_A = A[22:0];
			shifted_mantissa_B = B[22:0] >> -exp_diff;
		end else if (A[30:23] < B[30:23]) begin
			shifted_mantissa_A = A[22:0] >> exp_diff;
			shifted_mantissa_B = B[22:0];
		end else begin
			shifted_mantissa_A = A[22:0];
			shifted_mantissa_B = B[22:0];
		end
	end

	always @(A or B or shifted_mantissa_A or shifted_mantissa_B) begin
		if (A[31] == B[31]) begin
			new_sign = A[31];
		end else if (shifted_mantissa_A > shifted_mantissa_B) begin
			new_sign = A[31];
		end else begin
			new_sign = B[31];
		end
	end
endmodule
