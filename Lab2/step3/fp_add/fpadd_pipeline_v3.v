
/*
    The Floating Point Adder (fpadd_pipeline) module implements a pipelined floating-point adder.
    It takes two 32-bit floating-point numbers as input and produces a 32-bit floating-point result,
    following the IEEE 754 standard for single-precision floating-point representation.
    Both addition and subtraction operations are supported based on the signs of the inputs.

    ?Inputs:
        clk: Clock signal
        reset: Reset signal
        i_fpAdd_regA: First floating-point number (32 bits)
        i_fpAdd_regB: Second floating-point number (32 bits)
    ?Outputs:
        o_fpAdd_out: Result of the floating-point addition/subtraction (32 bits)
*/

module fpadd_pipeline (
	input clk,
    input reset,
    input [31:0] i_fpAdd_regA, 
    input [31:0] i_fpAdd_regB,  
	output [31:0] o_fpAdd_out
);
    // Inputs
    wire pl_signA, pl_signB;
    wire [7:0] pl_expA, pl_expB;
    wire [22:0] pl_mantA, pl_mantB;

    // Final Signals
    reg [7:0] finalExp;
    reg [22:0] finalMantissa;

    // Addition and Subtraction Signals
    wire [7:0] expDiff;
    reg [24:0] regsMantSum;
    wire [4:0] leadingZeroCount;

    // Larger and Smaller difference
    reg largerSign, smallerSign;
    reg [7:0] largerExp, smallerExp;
    reg [23:0] largerMant, smallerMant;
    wire [23:0] smallShiftMant;

    // Pipeline Signals
    wire [63:0] pipeOutS1;
    wire [32:0] pipeOutS2;
    wire [7:0] pl_largerExp;
    wire [24:0] pl_regsMantSum;
    
    // Pipe Stage 1
    pipeline #(.WIDTH(64)) 
        pipeline_inst (.clk(clk), .reset(reset), .in({i_fpAdd_regA, i_fpAdd_regB}), .out(pipeOutS1));

    assign {pl_signA, pl_expA, pl_mantA, pl_signB, pl_expB, pl_mantB} = pipeOutS1;

    always @(pl_signA or pl_signB or pl_expA or pl_expB or pl_mantA or pl_mantB) begin
        if ({pl_expA, pl_mantA} > {pl_expB, pl_mantB}) begin
            largerSign = pl_signA;
            largerExp = pl_expA;
            largerMant = {|pl_expA, pl_mantA};
            smallerSign = pl_signB;
            smallerExp = pl_expB;
            smallerMant = {|pl_expB, pl_mantB};
        end else begin
            largerSign = pl_signB;
            largerExp = pl_expB;
            largerMant = {|pl_expB, pl_mantB};
            smallerSign = pl_signA;
            smallerExp = pl_expA;
            smallerMant = {|pl_expA, pl_mantA};
        end
    end

    assign expDiff = largerExp - smallerExp;
    assign smallShiftMant = smallerMant >> expDiff;

    // Perform addition/subtraction based on sign
    always @(*) begin 
        if (largerSign ^ smallerSign) begin
            regsMantSum = largerMant - smallShiftMant;
        end else begin
            regsMantSum = largerMant + smallShiftMant;
        end
    end

    // Pipe Stage 2
    pipeline #(.WIDTH(33)) 
        pipeline_inst3 (.clk(clk), .reset(reset), .in({largerExp, regsMantSum}), .out(pipeOutS2));

    assign {pl_largerExp, pl_regsMantSum} = pipeOutS2;

	LZCounter LCZ_inst(.bits(pl_regsMantSum[23:0]), .leadingZeroCount(leadingZeroCount));

    // Normalize result
    always @(*) begin
        if (pl_regsMantSum[24]) begin // Overflow
            finalMantissa = pl_regsMantSum >> 1;
            finalExp = pl_largerExp + 1;
        end else if (|pl_regsMantSum) begin
            finalMantissa = pl_regsMantSum << leadingZeroCount;
            finalExp = pl_largerExp - leadingZeroCount;
        end else begin
            finalMantissa = 23'b0;
			finalExp = 8'b0;
        end
    end
    
    // Pipe Stage 3
    pipeline #(.WIDTH(32)) 
        pipeline_inst2 (.clk(clk), .reset(reset), .in({largerSign, finalExp, finalMantissa}), .out(o_fpAdd_out));
endmodule

