
/*
   The SSD Driver (ssd_driver) module drives a single 7-segment display (via the char_decoder) by cycling between the
   high nibble and the low nibble of the input data every time the internal counter reaches a certain value. This allows 
   two distinct 4-bit segments from data_in to be displayed at a regular interval.

   ?Inputs:
      clk    : Clock signal
      reset  : Asynchronous reset signal
      data_in: 8-bit input containing two 4-bit segments to display

   ?Outputs:
      an : Enables the display of the selected 4-bit segment
      a, b, c, d, e, f, g : 7-segment display outputs, driven by the char_decoder submodule
*/

module ssd_driver (
    input clk, reset,
    input [7:0] data_in,
    output reg an,
    output a, b, c, d, e, f, g
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

always @(counter or data_in) begin
    if (counter[5] == 1'b1) begin
        an = 1'b0;
        decoder_input = data_in[3:0];
    end else begin
        an = 1'b1;
        decoder_input = data_in[7:4];
    end
end
    
endmodule