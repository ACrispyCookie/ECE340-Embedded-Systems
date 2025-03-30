
module pipeline #(
    parameter PIPE_LEN = 2
) (
    input clk,
    input reset,
    input [PIPE_LEN-1:0] in,
    output reg [PIPE_LEN-1:0] out
);
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            out <= 0;
        end else begin
            out <= in;
        end
    end
endmodule