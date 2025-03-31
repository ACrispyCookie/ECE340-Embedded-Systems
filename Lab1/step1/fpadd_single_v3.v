
/*
    The Floating Point Adder (fpadd_pipeline) module implements a single-cycle floating-point adder.
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

module fpadd (
	input clk,
    input reset,
    input [31:0] i_fpAdd_regA, 
    input [31:0] i_fpAdd_regB,  
	output reg[31:0] o_fpAdd_out
);
    // Inputs
    reg signA, signB;
    reg [7:0] expA, expB;
    reg [22:0] mantA, mantB;

    reg [7:0] finalExp;
    reg [22:0] finalMantissa;
    reg [24:0] regsMantSum;
    wire [7:0] expDiff;
    wire [4:0] leadingZeroCount;

    reg largerSign, smallerSign;
    reg [7:0] largerExp, smallerExp;
    reg [23:0] largerMant, smallerMant;
    wire [23:0] smallShiftMant;

    assign expDiff = largerExp - smallerExp;
    assign smallShiftMant = smallerMant >> expDiff;

	LZCounter LCZ_inst(.bits(regsMantSum[23:0]), .leadingZeroCount(leadingZeroCount));
    
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            signA <= 0;
            signB <= 0;
            expA <= 0;
            expB <= 0;
            mantA <= 0;
            mantB <= 0;
            o_fpAdd_out <= 0;
        end else begin
            signA <= i_fpAdd_regA[31];
            signB <= i_fpAdd_regB[31];
            expA <= i_fpAdd_regA[30:23];
            expB <= i_fpAdd_regB[30:23];
            mantA <= i_fpAdd_regA[22:0];
            mantB <= i_fpAdd_regB[22:0];
            o_fpAdd_out <= {largerSign, finalExp, finalMantissa};
        end
    end

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
        if (regsMantSum[24]) begin // Overflow
            finalMantissa = regsMantSum >> 1;
            finalExp = largerExp + 1;
        end else if (|regsMantSum) begin
            finalMantissa = regsMantSum << leadingZeroCount;
            finalExp = largerExp - leadingZeroCount;
        end else begin
            finalMantissa = 23'b0;
			finalExp = 8'b0;
        end
    end
endmodule

