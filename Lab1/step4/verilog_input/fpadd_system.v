/*
 -----------------------------------------------------------------------------
 -- File           : fpadd_system.v
 -----------------------------------------------------------------------------
 */ 
 module fpadd_system (
   input clk,
   input reset, 
   input [1:0] mem_select,
   input [1:0] display_select,
   output reg [7:0] leds, 
   output an0, output a0, output b0, output c0, output d0, output e0, output f0, output g0,
   output an1, output a1, output b1, output c1, output d1, output e1, output f1, output g1
);

   wire [31:0] fp_out;
   wire [31:0] numA, numB;
   reg [7:0] ssd0_input, ssd1_input;

   wire reset_debounced;
   wire [1:0] display_select_debounced;
   wire [1:0] mem_select_debounced;
   wire [1:0] mem_select_pulse;
   
   input_debouncer reset_debounced_inst (.clk(clk), .input_bounce(reset), .debounced(reset_debounced));
   input_debouncer mem_select0_debouncer_inst (.clk(clk), .input_bounce(mem_select[0]), .debounced(mem_select_debounced[0]));
   input_debouncer mem_select1_debouncer_inst (.clk(clk), .input_bounce(mem_select[1]), .debounced(mem_select_debounced[1]));
   input_debouncer display_select0_debouncer_inst (.clk(clk), .input_bounce(display_select[0]), .debounced(display_select_debounced[0]));
   input_debouncer display_select1_debouncer_inst (.clk(clk), .input_bounce(display_select[1]), .debounced(display_select_debounced[1]));
   level_to_pulse l2p_inst1 (.clk(clk), .reset(reset), .level_input(mem_select_debounced[0]), .pulse_output(mem_select_pulse[0]));
   level_to_pulse l2p_inst2 (.clk(clk), .reset(reset), .level_input(mem_select_debounced[1]), .pulse_output(mem_select_pulse[1]));

   input_memory #(
      .INIT_00(32'h3f800000),
      .INIT_01(32'hbf800000),
      .INIT_02(32'hc2de8000),
      .INIT_03(32'h6b64b235),
      .INIT_04(32'h2ac49214),
      .INIT_05(32'hbfc66666),
      .INIT_06(32'hc565ee8b),
      .INIT_07(32'h447a4efa)
   )
   memA(.clk(clk), .reset(reset_debounced), .scroll_button(mem_select_pulse[0]), .selected_num(numA));
   
   input_memory #(
      .INIT_00(32'h40000000),
      .INIT_01(32'h3f800000),
      .INIT_02(32'h45155e00),
      .INIT_03(32'h6ac49214),
      .INIT_04(32'h6ac49214),
      .INIT_05(32'h3fc7ae14),
      .INIT_06(32'h4565ee8a),
      .INIT_07(32'h00000000)
   )
   memB(.clk(clk), .reset(reset_debounced), .scroll_button(mem_select_pulse[1]), .selected_num(numB));
   
   // Instantiate the FP adder
   fpadd_pipeline fpadd_pipeline_inst(.clk(clk), .reset(reset), .i_fpAdd_regA(numA), .i_fpAdd_regB(numB), .o_fpAdd_out(fp_out));

   always @(display_select_debounced) begin
      case (display_select_debounced)
         2'b01: {ssd0_input, ssd1_input, leds} = {numA[31:24], numA[23:16], numA[7:0]};
         2'b10: {ssd0_input, ssd1_input, leds} = {numB[31:24], numB[23:16], numB[7:0]};
         default: {ssd0_input, ssd1_input, leds} = {fp_out[31:24], fp_out[23:16], fp_out[7:0]};
      endcase
   end
   
   // Instantiate the 7segment display output 0 
   ssd_driver ssd0(.clk(clk), .reset(reset_debounced), .data_in(ssd0_input), .an(an0), .a(a0), .b(b0), .c(c0), .d(d0), .e(e0), .f(f0), .g(g0));
   
   // Instantiate the 7segment display output 1
   ssd_driver ssd1(.clk(clk), .reset(reset_debounced), .data_in(ssd1_input), .an(an1), .a(a1), .b(b1), .c(c1), .d(d1), .e(e1), .f(f1), .g(g1));

endmodule
