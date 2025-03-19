module ssd_driver (
    input clk, reset,
    input [7:0] data_in,
    output reg an,
    output reg a, b, c, d, e, f, g
);

reg [5:0] counter;
reg [3:0] decoder_input;
char_decoder decoder (
    .data_in(decoder_input),
    .a(a),
    .b(b),
    .c(c),
    .d(d),
    .e(e),
    .f(f),
    .g(g)
);

always @(posedge clk or posedge reset) begin
    if (reset) begin
        counter <= 6'b0;
    end else begin
        counter <= counter + 6'b1;
    end
end

always @(counter) begin
    if (counter[5] == 1'b1) begin
        an = 1'b1;
        decoder_input = data_in[3:0];
    end else begin
        an = 1'b0;
        decoder_input = data_in[7:4];
    end
end
    
endmodule