/*
 -----------------------------------------------------------------------------
 -- File           : fpadd_system.v
 -----------------------------------------------------------------------------
 */ 
 module fpadd_system_simple (
   input clk,
   input reset, 
   input button, 
   output wire [7:0] leds, 
   output an0, output a0, output b0, output c0, output d0, output e0, output f0, output g0,
   output an1, output a1, output b1, output c1, output d1, output e1, output f1, output g1
);

   wire [31:0] fp_out;
   wire [63:0] num;
   wire button_debounced, button_pulse;
   wire reset_debounced;
   
   input_debouncer reset_debounced_inst (.clk(clk), .signal(reset), .signal_debounced(reset_debounced));
   input_debouncer mem_select0_debouncer_inst (.clk(clk), .signal(button), .signal_debounced(button_debounced));
   level_to_pulse l2p_inst2 (.clk(clk), .reset(reset_debounced), .level_input(button_debounced), .pulse_output(button_pulse));

    input_memory #(
       .INIT_00(64'h3f800000_40000000),
       .INIT_01(64'hbf800000_3f800000),
       .INIT_02(64'hc2de8000_45155e00),
       .INIT_03(64'h6b64b235_6ac49214),
       .INIT_04(64'h2ac49214_6ac49214),
       .INIT_05(64'hbfc66666_3fc7ae14),
       .INIT_06(64'hc565ee8b_4565ee8a),
       .INIT_07(64'h447a4efa_00000000)
    )
    memA(.clk(clk), .reset(reset_debounced), .scroll_button(button_pulse), .selected_num(num));

   // Instantiate the FP adder
   fpadd_pipeline fpadd_pipeline_inst(.clk(clk), .reset(reset_debounced), .reg_A(num[63:32]), .reg_B(num[31:0]), .out(fp_out));

   assign leds = fp_out[7:0];
   
   // Instantiate the 7segment display output 0 
   ssd_driver ssd0(.clk(clk), .reset(reset_debounced), .data_in(fp_out[31:24]), .an(an0), .a(a0), .b(b0), .c(c0), .d(d0), .e(e0), .f(f0), .g(g0));
   
   // Instantiate the 7segment display output 1
   ssd_driver ssd1(.clk(clk), .reset(reset_debounced), .data_in(fp_out[23:16]), .an(an1), .a(a1), .b(b1), .c(c1), .d(d1), .e(e1), .f(f1), .g(g1));

endmodule
