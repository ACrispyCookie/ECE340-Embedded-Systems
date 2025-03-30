
/*
    The Pipeline module is a simple pipeline register that captures the input data on the rising edge of the clock
    and holds it until the next clock cycle.

    !Notice: This module can cause time violations when used alongside the concatenation operator.
    !Recommended usage example:

    * pipeline #(.WIDTH(64)) 
    *     pipeline_inst (.clk(clk), .reset(reset), .in({input_A, input_B}), .out(out_C));
    * assign {reg_A, reg_B} = out_C;

    !Avoid example:
    
    * pipeline #(.WIDTH(64)) 
    *     pipeline_inst (.clk(clk), .reset(reset), .in({input_A, input_B}), .out({reg_A, reg_B, ...}));

    ?Input:
        clk: Clock signal
        reset: Reset signal (active high)
        in: Input data of specified width
    ?Output:
        out: Output data of specified width, captured on the rising edge of the clock
*/

module pipeline #(
    parameter WIDTH = 64
) (
    input clk,
    input reset,
    input [WIDTH-1:0] in,
    output reg [WIDTH-1:0] out
);
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            out <= 0;
        end else begin
            out <= in;
        end
    end
endmodule
