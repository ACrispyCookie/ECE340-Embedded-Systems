
/*
   The Floating Point Adder System (fpadd_system) module integrates the floating-point adder and the 7-segment display driver.
   It takes two floating-point numbers as input, performs addition using the fpadd_pipeline module, and displays the result on
   two 7-segment displays, and the lower 8 bits of the result on LEDs.

   ?Inputs:
      clk: Clock signal
      reset: Reset signal
   ?Outputs:
      leds: 8 LEDs to display the lower 8 bits of the result
      an1, a1, b1, c1, d1, e1, f1, g1: 7-segment display signals for the second display
      an0, a0, b0, c0, d0, e0, f0, g0: 7-segment display signals for the first display
*/

 module fpadd_system (input clk,
                      input reset,
                      input enable,
                      input [31:0] A,
                      input [31:0] B,
                      output [7:0] leds, 
                      output an0, output a0, output b0, output c0, output d0, output e0, output f0, output g0,
                      output an1, output a1, output b1, output c1, output d1, output e1, output f1, output g1);

   wire [31:0] fp_out;
   reg [31:0] regA, regB;
   
   always @(posedge clk or posedge reset) begin
       if (reset) begin 
           regA <= 32'b0;
           regB <= 32'b0;
       end else if (enable) begin
           regA <= A;
           regB <= B;
       end
   end
    
   fpadd_pipeline fpadd_pipeline_inst(.clk(clk), .reset(reset), .i_fpAdd_regA(regA), .i_fpAdd_regB(regB), .o_fpAdd_out(fp_out));

   assign leds = fp_out[7:0];
   
   ssd_driver ssd0(.clk(clk), .reset(reset), .data_in(fp_out[31:24]), .an(an0), .a(a0), .b(b0), .c(c0), .d(d0), .e(e0), .f(f0), .g(g0));
   
   ssd_driver ssd1(.clk(clk), .reset(reset), .data_in(fp_out[23:16]), .an(an1), .a(a1), .b(b1), .c(c1), .d(d1), .e(e1), .f(f1), .g(g1));

endmodule
