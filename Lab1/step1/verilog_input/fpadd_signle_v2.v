module fpadd ( 
    input clk,
    input reset,
    input [31:0] i_fpAdd_regA, 
    input [31:0] i_fpAdd_regB,  
    output reg [31:0] o_fpAdd_out
);
    wire [4:0] leadingZeroCount;
    reg signA, signB;
    reg finalSign;
    reg [7:0] expA, expB;
    reg [7:0] expOut, finalExp;
    reg [22:0] rawMantA, rawMantB;
    reg [23:0] shiftedMantA, shiftedMantB;
    reg [23:0] mantA, mantB;
    reg [24:0] regsMantSum;
    reg [7:0] expDiff;
    wire [1:0] isZero;
    reg [22:0] finalMantissa;

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
            mantA <= {1'b1, i_fpAdd_regA[22:0]};
            mantB <= {1'b1, i_fpAdd_regB[22:0]};
            o_fpAdd_out <= {finalSign, finalExp, finalMantissa};
        end
    end

    // Exponent Calculations
    always @(*) begin
        if (expA > expB) begin
            expDiff = expA - expB;
            shiftedMantA = mantA;
            shiftedMantB = mantB >> expDiff;
            expOut = expA;
        end else begin
            expDiff = expB - expA;
            shiftedMantA = mantA >> expDiff;
            shiftedMantB = mantB;
            expOut = expB;
        end
    end

    // Perform addition/subtraction based on sign
    always @(*) begin 
        if (signA == signB) begin
            regsMantSum = shiftedMantA + shiftedMantB;
            finalSign = signA;
        end else begin
            if (shiftedMantA > shiftedMantB) begin
                regsMantSum = shiftedMantA - shiftedMantB;
                finalSign = signA;
            end else begin
                regsMantSum = shiftedMantB - shiftedMantA;
                finalSign = signB;
            end
        end
    end

    always @(*) begin
        // Normalize result
        if (regsMantSum[24]) begin // Overflow
            finalMantissa = regsMantSum >> 1;
            finalExp = expOut + 1;
        end else if (|regsMantSum) begin
            finalMantissa = regsMantSum << leadingZeroCount;
            finalExp = expOut - leadingZeroCount;
        end else begin
            finalMantissa = 23'b0;
			finalExp = 8'b0;
        end
    end
endmodule